//---------------------------------------------------------------------------
//--
//--	FICHIER	: vCardFile.h
//--
//--	AUTEUR	: Jérôme Henry-Barnaudière - JHB
//--
//--	PROJET	: ldap2File
//--
//---------------------------------------------------------------------------
//--
//--	DESCRIPTION:
//--
//--			Définition de la classe vCardFile
//--			Génération d'un fichier au format VCARD 3.0
//--
//---------------------------------------------------------------------------
//--
//--	MODIFICATIONS:
//--	-------------
//--
//--	08/02/2021 - JHB - Version 21.2.2
//--						+ Création
//--
//--	25/04/2021 - JHB - Version 21.4.12
//--
//---------------------------------------------------------------------------

#ifndef __LDAP_2_FILE_VCARD_OUTPUT_FILE_h__
#define __LDAP_2_FILE_VCARD_OUTPUT_FILE_h__

#include "textFile.h"

//----------------------------------------------------------------------
//--
//-- Constantes publiques
//--
//----------------------------------------------------------------------

#define VCARD_ASSIGN_OP			":"
#define VCARD_VALUE_SEP			";"

// Encadrement de la fiche
#define VCARD_BEGIN				"BEGIN"
#define VCARD_END				"END"
#define VCARD_VCARD				"VCARD"

// Attributs utilisés
#define VCARD_ORGANISATION		"ORG"
#define VCARD_ADDR				"ADR;type=WORK"
#define VCARD_FULLNAME			"N"
#define VCARD_FORMATED_NAME		"FN"
#define VCARD_TITLE				"TITLE"
#define VCARD_EMAIL				"EMAIL;type=INTERNET"
#define VCARD_MOBILE			"TEL;type=CELL;type=VOICE"
#define VCARD_TELEPHONENUMBER	"TEL;type=WORK;type=VOICE"


//----------------------------------------------------------------------
//--
//-- Définition de la classe
//--
//----------------------------------------------------------------------

class vCardFile : public textFile
{
	// Méthodes publiques
	//
public:

	// Construction
	vCardFile(const LPOPFI fileInfos, columnList* columns, confFile* parameters);

	// Destruction
	virtual ~vCardFile() {}

	// Création du fichier
	virtual bool create();

	// Paramètres spécifiques
	virtual bool getOwnParameters();

	// Enregistrement de la ligne courante
	virtual bool saveLine(bool header = false, LPAGENTINFOS agent = NULL);

	// Ajout d'une valeur (avec changement de colonne)
	virtual bool add(string& value)
	{ return false; }
	virtual bool add(deque<string>& values)
	{ return false; }

	// Ajout d'une valeur dans une colonne précise
	virtual bool addAt(size_t colIndex, string& value);
	virtual bool addAt(size_t colIndex, deque<string>& values);

	// Suppression d'une valeur
	virtual bool removeAt(size_t colIndex);

	// Remplacement d'une valeur
	virtual bool replaceAt(size_t colIndex, string& singleValue);

	// Sauvegarde / fermeture
	virtual bool close();

	// Méthodes privées
	//
private:

	// Un attrribut LDAP multivalué (ou pas ...)
	typedef struct tagLDAPATTRIBUTE
	{
		// Constructions
		tagLDAPATTRIBUTE(string& attrName, string& attrValue){
			name_ = attrName;
			values_.push_back(attrValue);
		}

		// Destruction
		virtual ~tagLDAPATTRIBUTE() {}

		// Son nom ...
		string name()
		{ return name_; }

		// Recherche d'une valeur
		bool exists(string& attrValue);

		// Ajout dune valeur
		void add(string& value) {
			if (!exists(value)) {
				// Pas déja en mémoire ...
				values_.push_back(value);
			}
		}

		// Je ne veux que la première valeur
		string value() {
			return (values_.size()?*(values_.begin()):"");
		}

		// Nettoyage (ie. suppression de toutes les valeurs)
		void clean() {
			values_.clear();
		}

		// Nombre d'éléments
		size_t size() {
			return values_.size();
		}

		string			name_;			// Nom de l'attribut
		list<string>	values_;		// Les valeurs (à priori non vides)
	}LDAPATTRIBUTE;

	// Toutes les informations d'un agent (ie. une "ligne" dans la logique fichier plat / CSV)
	class vCardUserDatas
	{
	public:
		// Construction
		vCardUserDatas(){}

		// Destruction
		virtual ~vCardUserDatas();

		// Nombre d'éléments
		size_t size() {
			return attributes_.size();
		}

		// Nettoyage de la liste
		void clean();

		// Recherche d'un attribut par son nom
		LDAPATTRIBUTE* findAttribute(string& attrName);
		LDAPATTRIBUTE* findAttribute(const char* attrName) {
			string value(IS_EMPTY(attrName)?"":attrName);
			return findAttribute(value);
		}

		// Recherche d'un attribut en fonction du tuple (nom, valeur)
		LDAPATTRIBUTE* findAttribute(string& name, string& value);

		// Ajout d'un attribut complet
		LDAPATTRIBUTE* add(LDAPATTRIBUTE* attr);

		// Ajout d'un attribut sans valeur ...
		LDAPATTRIBUTE* add(string& attrName, bool force = false){
			return (attrName.length() ? newAttribute(attrName, "", force) : NULL);
		}


		// Ajout d'un attribut et de sa valeur
		LDAPATTRIBUTE* newAttribute(string& attrName, string& attrValue, bool force = false);
		LDAPATTRIBUTE* newAttribute(string& attrName, const char* attrValue, bool force = false) {
			string value(IS_EMPTY(attrValue) ? "" : attrValue);
			return newAttribute(attrName, value, force);
		}
		LDAPATTRIBUTE* newAttribute(const char* attrName, const char* attrValue, bool force = false){
			if (!IS_EMPTY(attrName)){
				string name(attrName);
				string val = (IS_EMPTY(attrValue) ? "" : attrValue);
				return newAttribute(name, val, force);
			}
		}

		// Accès
		LDAPATTRIBUTE* operator [](size_t index);

	protected:
		deque<LDAPATTRIBUTE*>	attributes_;		// Liste des attributs
	};
	
	// Nouvelle ligne vierge
	void _newLine() {
		// Une ligne de +
		_incLines();
		
		// Nettoyage des attributs
		attributesToSave_.clean();
	}

	// Sauvegarde d'un attribut monovalué
	void _attribute2VCARD(const char* szName, const char* szValue){
		if (!IS_EMPTY(szName) && !IS_EMPTY(szValue)) {
			string value(encoder_.toUTF8(szValue));		// VCARD est en UTF8
			file_ << szName << VCARD_ASSIGN_OP << value << eol_;
		}
	}
	
	// Sauvegarde d'un attribut avec toutes ses valeurs
	void _attribute2VCARD(LDAPATTRIBUTE*, const char* szName = NULL);


	// Données membres privées
	//
private:
	charUtils					encoder_;			// Gestion de l'encodage des caractères

	vCardUserDatas				attributesToSave_;	// Attributs à sauvegarder

	// Dans le fichier XML ...
	string						organisation_;
	vCardUserDatas				add2All_;			// Attributs ajoutés à tous les objets
	
	bool						newFile_;
};

#endif // #ifndef __LDAP_2_FILE_VCARD_OUTPUT_FILE_h__

// EOF
