//---------------------------------------------------------------------------
//--
//--	FICHIER	: SMTPClient.cpp
//--
//--	AUTEUR	: Jérôme Henry-Barnaudière
//--
//--	DATE	: 11/05/2021
//--
//--    COMPATIBILITE : Win32 | Linux (Fedora 33)
//--
//---------------------------------------------------------------------------
//--
//--	DESCRIPTION:
//--
//--		Implémentation des classes SMTPClient, recipient & recipients
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

#include "SMTPClient.h"

#include <malloc.h>
#include <random>

//---------------------------------------------------------------------------
//--
//--	Constantes internes
//--
//---------------------------------------------------------------------------

// Taille de la chaine "Boudary"
#define BOUNDARY_LEN				65

// Association extension(s) => type MIME
static const std::string EXT2MIME{ ".avi -video/ms-video "
									".css -text/css "
									".csv -text/csv "
									".doc -application/vnd.ms-word "
									".docx -application/vnd.openxmlformats-officedocument.wordprocessingml.document "
									".flv -video/x-flv "
									".gif -image/gif "
									".htm .html -text/html "
									".jpg .jpeg-image/jpeg "
									".mpeg -video/mpeg "
									".mp4 -video/mp4 "
									".odg -application/vnd.oasis.opendocument.graphics "
									".odp -application/vnd.oasis.opendocument.presentation "
									".ods -application/vnd.oasis.opendocument.spreadsheet "
									".odt -application/vnd.oasis.opendocument.text "
									".pdf -application/pdf "
									".png -image/png "
									".ppt -application/vnd.ms-powerpoint "
									".pptx -application/vnd.openxmlformats-officedocument.presentationml.presentation "
									".qt -video/quicktime "
									".tiff -image/tiff "
									".wmv -video/x-ms-wmv "
									".xls -application/vnd.ms-excel "
									".xlsx -application/vnd.openxmlformats-officedocument.spreadsheetml.sheet "
									".xml -text/xml "
									".xul -application/vnd.mozilla.xul+xml " };

// Caractères à utilise pour les générations aléatoires
static const std::string alphaNum{
	"0123456789"
	"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
	"abcdefghijklmnopqrstuvwxyz" };

//---------------------------------------------------------------------------
//--
//--	Implémentation des classes
//--
//---------------------------------------------------------------------------

namespace jhbCURLTools
{

	//---------------------------------------------------------------------------
	//--
	//--	jhbSMTP::SMTPClient
	//--
	//---------------------------------------------------------------------------

	// Construction
	SMTPClient::SMTPClient(const std::string &from, const std::string &nameFrom, const std::string &subject, const std::string &body)
	:object_{ subject }, body_{body}, from_{from, nameFrom, recipient::DEST_TYPE::FROM}
	{
		encoder_.sourceFormat(charUtils::SOURCE_FORMAT::ISO_8859_15, true);
		eol_ = encoder_.eol(charUtils::FORMAT_EOL::EOL_CRLF);
	}

	// Envoi du message
	//
	CURLcode SMTPClient::send(const std::string& addr, int port, const std::string &user, const std::string& password, bool tls)
	{
		std::stringstream ss;
		if (tls) {
			ss << IPPROTO_SMTP;
		}
		else {
			ss << IPPROTO_SMTPS;
		}
		ss << IPPROTO_URL_SEP << addr << ":" << port;
		std::string fURL;
		ss >> fURL;
		return send(fURL , user, password, tls);
	}

	CURLcode SMTPClient::send(const std::string &url, const std::string &userName, const std::string &password, bool tls)
	{

		if (!from_.isValid() ||		// Si l'adresse source n'est pas valide
			!recipients_.size()){	// ou aucun destinataire pour le message
			return CURLE_BAD_FUNCTION_ARGUMENT;
		}

		CURLcode ret(CURLE_OK);
		struct curl_slist* dests(NULL);

		CURL *curl = curl_easy_init();

		if (curl){
			// Génération du message
			StringData textData { _generate() };

			// Transaction avec le serveur SMTP
			curl_easy_setopt(curl, CURLOPT_USERNAME, userName.c_str());
			curl_easy_setopt(curl, CURLOPT_PASSWORD , password.c_str());
			curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

			curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
			curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

			if (tls){
				curl_easy_setopt(curl, CURLOPT_USE_SSL, (long)CURLUSESSL_TRY);
				//curl_easy_setopt(curl, CURLOPT_CAINFO, "/path/to/certificate.pem");
			}

			// L'emeteur
			curl_easy_setopt(curl, CURLOPT_MAIL_FROM, (const char*)from_);

			// Tous les destinataires
			//
			for (recipientsIterator it = recipients_.begin(); it!=recipients_.end(); it+=1){
				dests = curl_slist_append(dests,  (const char*)(*it));
			}
			curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, dests);

			curl_easy_setopt(curl, CURLOPT_READFUNCTION, _read);
			curl_easy_setopt(curl, CURLOPT_READDATA, &textData);
			curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

#ifdef _DEBUG
			curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
#endif // #ifdef _DEBUG

			ret = curl_easy_perform(curl);

			curl_slist_free_all(dests);
			curl_easy_cleanup(curl);
		}

		return ret;
	}

	// Génération du message
	//
	std::string SMTPClient::_generate()
	{
		std::string ret;

		// Entête du message
		//
		ret = _addLine("Date", _dateTimeNow());
		ret += _addLine("From", from_);
		ret += _addLine("To", recipient::DEST_TYPE::TO);
		ret += _addLine("Cc", recipient::DEST_TYPE::CC);
		ret += _addLine("Bcc", recipient::DEST_TYPE::BCC);
		ret += _addLine("Subject", object_);
		ret += _addLine("Message-ID", _messageId());

		// Définition du séparateur
		bool multiparted(false);
		std::string separator = _boundary();
		ret += _addLine("Content-Type", "multipart/mixed;");
		ret += " boundary=\"" + separator + "\"" + eol_;
		ret += _addLine("MIME-Version", "1.0");

		// Corps
		if (body_.size()){
			ret += eol_ + "--" + separator + eol_;
			ret += _addLine("Content-Type", "text/plain; charset = \"utf-8\"");
			ret += _addLine("Content-Transfer-Encoding", "base64") + eol_;
			ret += encoder_.text2Base64(body_);

			multiparted = true;
		}

		// PJ(s)
		if (attachments_.size()){
			std::string header, buffer, shortName;
			size_t pos;

			for (std::vector<std::string>::iterator it = attachments_.begin(); it != attachments_.end(); it++){
				// Génération du buffer pour le fichier
				buffer = encoder_.file2Base64((*it));
				if (buffer.size()){
					// Le fichier existe et le buffer a été crée
					multiparted = true;

					// Nom court du fichier
					shortName = (*it);
#ifdef _WIN32
					pos = shortName.rfind("\\");
#else
					pos = shortName.rfind("/");
#endif // std::string shortName

					if (shortName.npos != pos){
						shortName = shortName.substr(pos + 1);
					}

					// Ajout au buffer
					header = _fileName2MIMEType((*it));
					header += "; name=\"";
					header += shortName;
					header += "\"";

					ret += eol_ + "--" + separator + eol_;
					ret += _addLine("Content-Type", header);
					ret += _addLine("Content-Transfer-Encoding", "base64") + eol_;
					ret += buffer;
				}
			}
		}

		// Fin du message
		if (multiparted){
			ret +=  "--" + separator + "--" + eol_ + eol_;
		}

		return ret;
	}

	// Date du jour
	//
	std::string SMTPClient::_dateTimeNow()
	{
		const int RFC5322_TIME_LEN = 32;

		std::string ret;
		ret.resize(RFC5322_TIME_LEN);

		time_t tt;

#ifdef _MSC_VER
		time(&tt);
		tm *t = localtime(&tt);
#else
		tm tv, *t = &tv;
		tt = time(&tt);
		localtime_r(&tt, t);
#endif

		strftime(&ret[0], RFC5322_TIME_LEN, "%a, %d %b %Y %H:%M:%S %z", t);

		return ret;
	}

	// Copie du message dans le buffer mémoire de curl
	//
	size_t SMTPClient::_read(void *ptr, size_t size, size_t nmemb, void *userp)
	{
		// Pointeur sur mon buffer mémoire
		StringData *text(reinterpret_cast<StringData *>(userp));

		// Vérification des paramètres
		if (NULL != text && NULL != ptr && 0 != nmemb*size){
			size_t bytesLeft = text->size() - text->offset_;

			// Nombre d'octets àˆ transfŽrer
			// soit ((size*nmemb)) octets soit ce qu'il reste effectivement ˆà copier
			size_t transfer(((size*nmemb)>=bytesLeft)?bytesLeft:(size*nmemb));

			// Copie
			memcpy(ptr, text->buffer_.c_str() + text->offset_, transfer);

			// Mise àˆ jour des pointeurs
			text->offset_ += transfer;

			// Nombre d'octets copiéŽs
			return transfer;
		}

		// Plus rien ˆà retourner
		return 0;
	}

	// Création d'une "ligne"
	//
	std::string SMTPClient::_addLine(const char* token, const char* value)
	{
		std::string sValue(value ? value : "");
		return _addLine(token, sValue);
	}

	// Une ligne avec un seul destinataire
	//
	std::string SMTPClient::_addLine(const char* token, recipient& dest)
	{
		if (!dest.isValid()){
			return "";
		}

		std::string destAddr("");

		// Nom
		if (dest.name().size()){
			destAddr = encoder_.toMIMEUTF8((const std::string&)dest.name(), charUtils::MIME_ENCODE::BASE64)+ " ";
		}

		// ajout de l'adresse
		destAddr += "<";
		destAddr += (const char*)dest;
		destAddr += ">";

		// Génération de la "ligne" en fonction de la taille max.
		return _addLine(token, destAddr);
	}

	// Une ligne avec une liste de destinataires
	//
	std::string SMTPClient::_addLine(const char* token, recipient::DEST_TYPE destType)
	{
		std::string out("");
		bool first(true);
		for (recipientsIterator it = recipients_.begin(destType); it != recipients_.end(); it+=1){
			// Séparateur
			if (first){
				// Pas au début ...
				first = false;
			}
			else{
				out += ", ";
			}

			// Nom
			if ((*it).name().size()){
				out += encoder_.toMIMEUTF8((*it).name(), charUtils::MIME_ENCODE::BASE64) + " ";
			}

			// Ajout de l'adresse
			out += "<";
			out += (const char*)(*it);
			out += ">";
		}

		// Génération de la "ligne" en fonction de la taille max.
		return (out.size()?_addLine(token, out):"");
	}

	// ID unique du message
	//
	std::string SMTPClient::_messageId() const
	{
		const size_t MESSAGE_ID_LEN = 37;
		tm t;
		time_t tt;
		time(&tt);
#ifdef _MSC_VER
		gmtime_s(&t, &tt);
#else
		gmtime_r(&tt, &t);
#endif
		std::string ret;
		ret.resize(MESSAGE_ID_LEN);
		size_t datelen = std::strftime(&ret[0], MESSAGE_ID_LEN, "%Y%m%d%H%M%S", &t);
		std::mt19937 gen;
		std::uniform_int_distribution<> dis(0, alphaNum.length() - 1);
		std::generate_n(ret.begin() + datelen, MESSAGE_ID_LEN - datelen, [&]() { return alphaNum[dis(gen)]; });
		ret += from_.domain();
		return ret;
	}

	// Séparateur (frontière) de contenu
	std::string SMTPClient::_boundary()
	{
		tm t;
		time_t tt;
		time(&tt);
#ifdef _MSC_VER
		gmtime_s(&t, &tt);
#else
		gmtime_r(&tt, &t);
#endif

		std::stringstream ss;
		ss << "=jhb_" << t.tm_hour << t.tm_min << t.tm_sec;
		std::string out;
		ss >> out;
		size_t len(out.size());
		out.resize(BOUNDARY_LEN);

		std::mt19937 gen;
		std::uniform_int_distribution<> dis(0, alphaNum.length() - 1);
		std::generate_n(out.begin() + len, BOUNDARY_LEN - len, [&]() { return alphaNum[dis(gen)]; });
		return out;
	}

	// Type MIME associé à une extension
	//
	std::string SMTPClient::_fileName2MIMEType(const std::string& fileName)
	{
		// Par défaut ...
		std::string mime("application/octet-stream");

		size_t to, pos = fileName.rfind(".");
		if (fileName.npos != pos && pos != fileName.size()){
			std::string ext = encoder_.strlwr(fileName.substr(pos).c_str());
			ext += " ";

			// recherche de l'extension
			std::string mimes(EXT2MIME);
			if (mimes.npos != (pos = mimes.find(ext))){
				to = mimes.find("-", pos);
				pos = mimes.find(" ", to++);
				mime = mimes.substr(to, (pos == mimes.npos) ? pos : pos - to);
			}
		}

		return mime;
	}

	//---------------------------------------------------------------------------
	//--
	//--	jhbSMTP::recipient
	//--
	//---------------------------------------------------------------------------

	// L'adresse est-elle valide ?
	//
	bool recipient::isValid()
	{
		if (initDone_){
			// Déja vérifiée
			return valid_;
		}

		// Vérification
		initDone_ = true;

		// Au moins l'adresse est renseignée
		if (!address_.size() || address_.npos == address_.find("@")){
			valid_ = false;
			return false;
		}

		// Format de l'adresse

		// oui
		valid_ = true;
		return true;
	}

	//---------------------------------------------------------------------------
	//--
	//--	jhbSMTP::recipients
	//--
	//---------------------------------------------------------------------------

	// Ajout d'un destinataire à la liste
	//
	bool recipients::add(const char* destAddr, const char* destName, recipient::DEST_TYPE destType)
	{
		// Vérification de l'adresse
		std::string addr(destAddr?destAddr:"");
		if (!addr.size() || destType == recipient::DEST_TYPE::ANY){
			return false;
		}

		// Nom "humain"
		std::string name(destName?destName:"");

		recipient dest(addr, name, destType);

		// Est-elle bien formée ?
		if (!dest.isValid()){
			return false;
		}

		// Ajout
		recipients_.push_back(dest);

		// Fait
		return true;
	}

	//---------------------------------------------------------------------------
	//--
	//--	jhbSMTP::recipientsIterator
	//--
	//---------------------------------------------------------------------------

	// On avance jusqu'au prochain destinataire du type recherché
	//
	void recipientsIterator::_forward(bool checkCurrent)
	{
		// Déja à la fin ?
		if (pos_ == recipients_->size()){
			return;
		}

		// Dois je vérifier si la pos. actuelle est correcte ?
		if (checkCurrent){
			if (dest_ == recipient::DEST_TYPE::ANY ||
				recipients_->at(pos_).destType() == dest_){
				// Pas besoin d'avancer
				return;
			}
		}
		else{
			pos_++;
		}

		// On avance jusqu'au prochain destinataire de ce type
		while (pos_ < recipients_->size() && dest_ != recipient::DEST_TYPE::ANY && recipients_->at(pos_).destType() != dest_){
			pos_++;
		}

	}

} // namespace jhbCURLTools

// EOF
