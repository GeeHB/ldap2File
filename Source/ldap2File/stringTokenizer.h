//---------------------------------------------------------------------------
//--
//--	FICHIER	: stringTokenizer.h
//--
//--	AUTEUR	: J�r�me Henry-Barnaudi�re - JHB
//--
//--	PROJET	: ldap2File
//--
//--    COMPATIBILITE : Win32 | Linux (Fedora 33)
//--
//---------------------------------------------------------------------------
//--
//--	DESCRIPTION:
//--
//--		D�finition de l'objet stringTokenizer pour le remplacement de "token"
//--		dans une chane de caract�res
//--
//---------------------------------------------------------------------------
//--
//--	MODIFICATIONS:
//--	-------------
//--
//--	17/05/2021 - JHB - Cr�ation
//--
//--	27/05/2021 - JHB - Version 21.5.7
//--
//---------------------------------------------------------------------------

#ifndef __LDAP_2_STRING_TOKENIZER_h__
#define __LDAP_2_STRING_TOKENIZER_h__   1

#include "sFileSystem.h"
#include "charUtils.h"

// stringTokenizer
//	Classe de base pour le remplacement dans les cha�nes de caract�res
//
class stringTokenizer
{
	// M�thodes publiques
public:

	// Constrcution
	stringTokenizer() {}

	// Destruction
	virtual ~stringTokenizer()
	{ clear(); }

	// Effacement de la liste des token
	void clear() {
		for (std::list<LPSINGLEITEM>::iterator i = items_.begin(); i != items_.end(); i++) {
			if ((*i)) {
				delete (*i);
			}
		}

		items_.clear();
	}

	// Remplacement d'un token par sa valeur
	//
	void replace(std::string& source, const char* token, const char* value) {
		std::string sToken(token), sValue(value);
		replace(source, sToken, sValue);
	}
	void replace(std::string& source, std::string& token, std::string& value) {
		size_t tSize;
		if (source.size() && (tSize = token.size())) {
			size_t from(0);
			// Tant que le token est trouv� ...
			while (source.npos != (from = source.find(token, from))) {
				source.replace(from, tSize, value);		// ... il est remplac� par sa valeur (ou rien)
				from++;
			}
		}
	}

	// Ajout d'une tuple (token, valeur) pour remplacement ult�rieur
	//
	void addToken(const char* token, const char* value, bool handleSpace = false) {
		std::string sToken(token), sValue(value);
		addToken(sToken, sValue, handleSpace);
	}
	void addToken(const char* token, int value, int digits = 0) {
		std::string sToken(token);
		std::string sNum(charUtils::itoa(value, 10, digits));
		addToken(sToken, sNum);
	}
	void addToken(std::string& token, std::string& value, bool handleSpace = false) {

		std::string newValue("");
		if (handleSpace && 0 != value.find(' ')){
		    // Gestion des espaces
		    newValue = "\"";
		    newValue += value;
		    newValue += "\"";
		}
		else{
		    newValue = value;
		}

		// Le nom ne peut pas �tre vide ...
		if (token.size()) {
			LPSINGLEITEM prev = _findByName(token);
			if (NULL != prev) {
				// Mise � jour de la valeur
				prev->value_ = newValue;
			}
			else {
				if (NULL != (prev = new SINGLEITEM(token.c_str(), newValue.c_str()))) {
					// Nouvel �l�ment
					items_.push_back(prev);
				}
			}
		}
	}

	// Remplacement de tous les items
	//
	size_t replace(std::string& source) {
		size_t total(0);
		for (std::list<LPSINGLEITEM>::iterator i = items_.begin(); i != items_.end(); i++) {
			if ((*i)) {
				replace(source, (*i)->name_, (*i)->value_);
				total++;
			}
		}

		// Comobien de remplacements ?
		return total;
	}

	// M�thodes priv�es
	//
protected:

	// Un item de remplacement
	typedef struct tagSINGLEITEM {

		// Construction
		tagSINGLEITEM(const char* name, const char* value) {
			name_ = IS_EMPTY(name) ? "" : name;
			value_ = IS_EMPTY(value) ? "" : value;
		}

		std::string name_;
		std::string value_;

	}SINGLEITEM, *LPSINGLEITEM;

	// Recherche d'un item dans la liste en fonction de son nom
	//
	LPSINGLEITEM _findByName(std::string& token) {
		for (std::list<LPSINGLEITEM>::iterator i = items_.begin(); i != items_.end(); i++) {
			if ((*i) && (*i)->name_ == token) {
				// Trouv�
				return (*i);
			}
		}

		// Non trouv�
		return NULL;
	}

	// Donn�es membres
	//
protected:

	// Liste des �l�ments � remplacer
	std::list<LPSINGLEITEM>	items_;
};

#endif // __LDAP_2_STRING_TOKENIZER_h__

// EOF
