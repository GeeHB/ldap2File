//---------------------------------------------------------------------------
//--
//--	FICHIER	: stringTokenizer.h
//--
//--	AUTEUR	: Jérôme Henry-Barnaudière - JHB
//--
//--	PROJET	: ldap2File
//--
//--    COMPATIBILITE : Win32 | Linux (Fedora 33)
//--
//---------------------------------------------------------------------------
//--
//--	DESCRIPTION:
//--
//--		Définition de l'objet stringTokenizer pour le remplacement de "token" dans une 
//--		chaine de caractères
//--
//---------------------------------------------------------------------------
//--
//--	MODIFICATIONS:
//--	-------------
//--
//--	17/05/2021 - JHB - Création
//--
//--	14/05/2021 - JHB - Version 21.5.4
//--
//---------------------------------------------------------------------------

#ifndef __LDAP_2_STRING_TOKENIZER_h__
#define __LDAP_2_STRING_TOKENIZER_h__

#include "sFileSystem.h"
#include "charUtils.h"

// stringTokenizer
//	Classe de base pour le remplacement dans les chaînes de caractère
//
class stringTokenizer
{
	// Méthodes publiques
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
			// Tant que le token est trouvé ...
			while (source.npos != (from = source.find(token, from))) {
				source.replace(from, tSize, value);		// ... il est remplacé par sa valeur (ou rien)
				from++;
			}
		}
	}

	// Ajout d'une tuple (token, valeur) pour remplacement ultérieur
	//
	void addToken(const char* token, const char* value) {
		std::string sToken(token), sValue(value);
		addToken(sToken, sValue);
	}
	void addToken(const char* token, int value, int digits = 0) {
		std::string sToken(token);
		std::string sNum(charUtils::itoa(value, 10, digits));
		addToken(sToken, sNum);
	}
	void addToken(std::string& token, std::string& value) {
		// Le nom ne peut pas être vide ...
		if (token.size()) {
			LPSINGLEITEM prev = _findByName(token);
			if (NULL != prev) {
				// Mise à jour de la valeur
				prev->value_ = value;
			}
			else {
				if (NULL != (prev = new SINGLEITEM(token.c_str(), value.c_str()))) {
					// Nouvel élément
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

	// Méthodes privées
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
				// Trouvé
				return (*i);
			}
		}

		// Non trouvé
		return NULL;
	}

	// Données membres
	//
protected:

	// Liste des éléments à remplacer
	std::list<LPSINGLEITEM>	items_;
};

#endif // __LDAP_2_STRING_TOKENIZER_h__

// EOF