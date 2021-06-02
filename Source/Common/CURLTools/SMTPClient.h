//---------------------------------------------------------------------------
//--
//--	FICHIER	: SMTPClient.h
//--
//--	AUTEUR	: Jérôme Henry-Barnaudière
//--
//--	DATE	: 11/05/2021
//--
//--    COMPATIBILITE : Win32 | Linux (Fedora 34 et supérieures)
//--
//---------------------------------------------------------------------------
//--
//--	DESCRIPTION:
//--
//--		Définition des classes SMTPClient, recipient & recipients
//--		pour les envois de mails par SMTP
//--
//---------------------------------------------------------------------------
//--
//--	MODIFICATIONS:
//--	-------------
//--
//--	xx/xx/xxxx - V0 par Christopher w. Backen <immortal@cox.net>
//--				  - Classe CFastSmtp
//--
//--	xx/xx/xxxx - V1 - CSMTP - par Jakub Piwowarczyk
//--
//--	02/02/2015 - JHB - smtpClient - Corrections et normalisation
//--
//--	11/01/2017 - JHB - Version 3.0 - Devient jhbSMTP::message
//						+ Création d'un namespace jhbSMTP
//--					+ Compatible MacOS X / LINUX  / Windows - Basée sur curl
//--
//--	06/07/2020 - JHB - devient jhbCURLTools::SMTPClient
//--
//--	11/05/2021 - JHB - Corrections & compatibilité Win32 / Linux
//--
//---------------------------------------------------------------------------

#ifndef __JHB_CUPS_SMTP_CLIENT_h__
#define __JHB_CUPS_SMTP_CLIENT_h__      1

#include "CURLTools.h"

#include <charUtils.h>			// Encodages UTF8 / Base64

//---------------------------------------------------------------------------
//--
//--    Définition des classes
//--
//---------------------------------------------------------------------------

namespace jhbCURLTools{

	// Une adresse email formatée selon la RFC
	//
	class recipient
	{
	public:

		// Type de destinataire
		enum class DEST_TYPE {FROM = 0, SENDER, REPLAY_TO, TO, CC, BCC, ANY};

		// Constructeurs
		recipient(const std::string &email)
		: address_{email}, name_{""}, dest_{DEST_TYPE::TO}, valid_{false}, initDone_{false}
		{}
		recipient(const std::string &email, const std::string &displayName, DEST_TYPE destType)
		: address_{email}, name_{displayName}, dest_{destType}, valid_{false}, initDone_{false}
		{}
		recipient(const recipient& src)
		: address_{src.address_}, name_{src.name_}, dest_{src.dest_}, valid_{src.valid_}, initDone_{src.initDone_}
		{}

		// L'adresse est-elle valide ?
		bool isValid();

		// Type du destinataire
		DEST_TYPE destType()
		{ return dest_; }
		void setDestType(DEST_TYPE dest)
		{ dest_ = dest; }

		// Accès
		std::string domain() const
		{ return address_.substr(address_.find('@')); }
		explicit operator const char *() const
		{ return address_.c_str(); }
		std::string name()
		{ return name_; }

		// Flux
		friend std::ostream& operator<<(std::ostream &out, const recipient& email){
			// Retour formaté si le nom est renseigné
			return email.name_.size()?(out << email.name_ << " " << email.address_) : (out << email.address_);
		}

	// Données membres
	private:
		std::string	address_;	// L'adresse
		std::string	name_;		// Le nom du destinataire
		DEST_TYPE	dest_;		// Type de destinataire
		bool		valid_;
		bool		initDone_;
	};

	// Mon itérateur
	class recipientsIterator
	{
		public:
			// Construction sur un type de destinataire
			recipientsIterator(std::vector<recipient>* recList, recipient::DEST_TYPE dest, size_t pos)
			:pos_{pos}, dest_{dest}, recipients_{recList}
			{ if (pos<recList->size()) _forward(true); /* On se met sur le premier valide */ }
			recipientsIterator(const recipientsIterator& src)
			:pos_{src.pos_}, dest_{src.dest_}, recipients_{src.recipients_}
			{}

			// Accès
			recipient& operator*()
			{ return recipients_->at(pos_);}
			void first(recipient::DEST_TYPE dest)
			{ dest_ = dest; pos_=0; _forward(true); }

			// On avance
			recipientsIterator& operator+=(int anyValue){
				_forward();
				return *this;
			}

			recipientsIterator& last(){
				pos_ = recipients_->size();
				dest_ = recipient::DEST_TYPE::ANY;
				return *this;
			}

			// Comparaison
			bool operator==(const recipientsIterator& right)
			{ return _equal(right); }
			bool operator!=(const recipientsIterator& right)
			{ return !_equal(right); }

			// Méthodes privées
		private:
			// On avance jusqu'au prochain destinataire du type recherché
			void _forward(bool checkCurrent = false);

			// Comparaison
			bool _equal(const recipientsIterator& right){
				return (
					(	(dest_== right.dest_)
						|| (dest_ == recipient::DEST_TYPE::ANY)
						|| (right.dest_ == recipient::DEST_TYPE::ANY)
					)
					&& recipients_ == right.recipients_
					&& pos_ == right.pos_);
			}

		private:
			// Données membres
			size_t						pos_;			// Position dans la liste
			recipient::DEST_TYPE		dest_;
			std::vector<recipient>*		recipients_;	// Pointeur sur la liste
	};

	// Liste de destinataires
	//
	class recipients
	{
		public:
			recipients()
			:begin_(&recipients_, recipient::DEST_TYPE::ANY, 0), end_(&recipients_, recipient::DEST_TYPE::ANY, 0)
			{}
			virtual ~recipients()
			{}

			// Ajout
			bool add(const char* destAddr, const char* destName = NULL, recipient::DEST_TYPE destType = recipient::DEST_TYPE::TO);

			// Parcours
			recipientsIterator& begin(recipient::DEST_TYPE destType = recipient::DEST_TYPE::ANY){
				begin_.first(destType);
				return begin_;
			}
			recipientsIterator& end(){
				end_.last();
				return end_;
			}

			static bool validAddress(recipient& dest)
			{ dest.isValid(); return true; }

			size_t size()
			{ return recipients_.size();}

		// Données membres
		private:
			std::vector<recipient>	recipients_;
			recipientsIterator		begin_, end_;
	};

	// Définition de la classe SMTPClient
	//
	class SMTPClient
	{
	public:
		// Construction
		SMTPClient(const std::string &addrFrom, const std::string &nameFrom, const std::string &subject, const std::string &body);

		// Un destinataire ...
		bool addRecipient(const char* destAddr, const char* destName, recipient::DEST_TYPE destType)
		{ return recipients_.add(destAddr, destName, destType); }
		bool addRecipient(const char* destAddr, recipient::DEST_TYPE destType = recipient::DEST_TYPE::TO)
		{ return addRecipient(destAddr, NULL, destType); }

		// Une pièce jointe
		bool addAttachment(const std::string& fileName)
		{ attachments_.push_back(fileName); return true; }


		// Envoi du message
		CURLcode send(const std::string &url, int port, const std::string &user, const std::string& password, bool tls);
		CURLcode send(const std::string &url, const std::string &user, const std::string& password, bool tls);

		// Méthodes privées
		//
	private:

		struct StringData{
			std::string	buffer_;
			size_t		offset_;

			StringData(std::string &&m)
			: buffer_{ m }, offset_{0}
			{}
			StringData(std::string &m) = delete;
			size_t size()
			{ return buffer_.size(); }
		};

		// Contenu du "message" curl
		static size_t _read(void *ptr, size_t size, size_t nmemb, void *userp);  // Envoi
		std::string _generate();  // Génération du message

		// Création d'une "ligne"
		//		Une "ligne" est découpée en une ou plusieurs lignes dans le flux de sortie
		//
		std::string _addLine(const char* token, const std::string& value)
		{ return encoder_.createKeyValueLine(token, value, RFC5322_MAX_LINE_LEN, eol_); }
		std::string _addLine(const char* token, const char* value);
		std::string _addLine(const char* token, recipient& dest);
		std::string _addLine(const char* token, recipient::DEST_TYPE destType);

		// Date du jour
		static std::string _dateTimeNow();

		// Identifiant unique pour le message
		std::string _messageId() const;

		// Séparateur
		std::string _boundary();

		// Type MIME
		std::string _fileName2MIMEType(const std::string& fileName);

		// Données membres
		//
	private:

		// Message
		std::string					object_;
		std::string					body_;
		recipient					from_;
		std::string					eol_;

		// Mes destinataires
		recipients					recipients_;

		// Piéces jointes
		std::vector<std::string>	attachments_;

		// Encodeur
		charUtils					encoder_;
	};
}; // namespace jhbCURLTools

#endif // ifndef __JHB_CUPS_SMTP_CLIENT_h__

// EOF
