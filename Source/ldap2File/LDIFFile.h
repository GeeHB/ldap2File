//---------------------------------------------------------------------------
//--
//--	FICHIER	: LDIFFile.h
//--
//--	AUTEUR	: J�r�me Henry-Barnaudi�re - JHB
//--
//--	PROJET	: ldap2File
//--
//---------------------------------------------------------------------------
//--
//--	DESCRIPTION:
//--
//--			D�finition de la classe LDIFFile
//--			G�n�ration d'un fichier au format LDIF
//--
//---------------------------------------------------------------------------
//--
//--	MODIFICATIONS:
//--	-------------
//--
//--	05/04/2020 - JHB - Version 20.4.6
//--						+ Cr�ation
//--
//--	06/04/2021 - JHB - Version 21.4.10
//--
//---------------------------------------------------------------------------

#ifndef __LDAP_2_FILE_LDIF_OUTPUT_FILE_h__
#define __LDAP_2_FILE_LDIF_OUTPUT_FILE_h__

#include "textFile.h"

//----------------------------------------------------------------------
//--
//-- Constantes publiques
//--
//----------------------------------------------------------------------


//----------------------------------------------------------------------
//--
//-- D�finition de la classe
//--
//----------------------------------------------------------------------

class LDIFFile : public textFile
{
	// M�thodes publiques
	//
public:

	// Construction
	LDIFFile(const LPOPFI fileInfos, columnList* columns, confFile* parameters);

	// Destruction
	virtual ~LDIFFile() {}

	// Cr�ation du fichier
	virtual bool create();

	// Param�tres sp�cifiques
	virtual bool getOwnParameters();

	// Enregistrement de la ligne courante
	virtual bool saveLine(bool header = false, LPAGENTINFOS agent = NULL);

	// Ajout d'une valeur (avec changement de colonne)
	virtual bool add(string& value)
	{ return false; }
	virtual bool add(deque<string>& values)
	{ return false; }

	// Ajout d'une valeur dans une colonne pr�cise
	virtual bool addAt(size_t colIndex, string& value);
	virtual bool addAt(size_t colIndex, deque<string>& values);

	// Suppression d'une valeur
	virtual bool removeAt(size_t colIndex);

	// Remplacement d'une valeur
	virtual bool replaceAt(size_t colIndex, string& singleValue);

	// Sauvegarde / fermeture
	virtual bool close();

	// M�thodes priv�es
	//
private:

	// Un attrribut LDAP multivalu� (ou pas ...)
	typedef struct tagLDAPATTRIBUTE
	{
		// Constructions
		tagLDAPATTRIBUTE(string& attrName, string& attrValue){
			name_ = attrName;
			outputName_ = "";
			values_.push_back(attrValue);
		}

		tagLDAPATTRIBUTE(tagLDAPATTRIBUTE& other) {
			name_ = other.name_;

			for (list<string>::iterator it = other.values_.begin(); it != other.values_.end(); it++) {
				if ((*it).size()) {
					values_.push_back(*it);
				}
			}
		}

		// Destruction
		virtual ~tagLDAPATTRIBUTE() {}

		// Son nom ...
		string name()
		{ return outputName_.length() ? outputName_ : name_; }

		// Recherche d'une valeur
		bool exists(string& attrValue);

		// Ajout dune valeur
		void add(string& value) {
			if (!exists(value)) {
				// Pas d�ja en m�moire ...
				values_.push_back(value);
			}
		}

		// Nettoyage (ie. suppression de toutes les valeurs)
		void clean() {
			values_.clear();
		}

		// Nombre d'�l�ments
		size_t size() {
			return values_.size();
		}

		string			name_;			// Nom de l'attribut
		string			outputName_;	// Nom de sortie (permet de fusionner les attributs)
		list<string>	values_;		// Les valeurs (� priori non vides)
	}LDAPATTRIBUTE;

	// Toutes les informations d'un agent (ie. une "ligne" dans la logique fichier plat / CSV)
	class LDIFUserDatas /*: public agentInfos::agentDatas*/
	{
	public:
		// Construction
		LDIFUserDatas(){
			allowEmpty_ = false;
		}

		// Destruction
		virtual ~LDIFUserDatas();

		// Transfert du contenu dans un autre objet
		void transferIn(LDIFUserDatas& other);

		// Valeurs "nulles" accept�es ?
		void setAllowEmpty(bool allow = true) {
			allowEmpty_ = allow;
		}
		bool allowEmpty() {
			return allowEmpty_;
		}

		// Nombre d'�l�ments
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

		// Cet attribut sera � fusionner !
		LDAPATTRIBUTE* newFusionAttribute(string& attrName, string& fusionWith){
			LDAPATTRIBUTE* newAttr(newAttribute(attrName, "", true));
			if (NULL != newAttr) {
				// Mise � jour de non fusionn�
				newAttr->outputName_ = fusionWith;
			}

			return newAttr;
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

		// Acc�s
		LDAPATTRIBUTE* operator [](size_t index);

	protected:
		bool					allowEmpty_;		// Autorisation de cr�ation d'un attribut sans valeur
		deque<LDAPATTRIBUTE*>	attributes_;		// Liste des attributs
	};
	
	// Nouvelle ligne vierge
	void _newLine() {
		// Une ligne de +
		_incLines();
		
		// Nettoyage des attributs
		attributesToSave_.clean();

		// On remet les attributs pr�vu pour tous
		add2All_.transferIn(attributesToSave_);
	}

	// L'attribut est-il obligatoire
	bool _isMandatory(string& name);

	// Sauvegarde d'un attribut avec toutes ses valeurs
	void _attribute2LDIF(LDAPATTRIBUTE*);

	// Formatage d'une chaine pour LDIF (UTF8 + Base64 si n�cessaire) + d�coupage multi-ligne
	//		La chaine g�n�r�e est au format [key][affectation][valeur encod�e]
	string _string2LDIF(const char* key, const char* source){
        if (IS_EMPTY(key)) {
			return "";
		}
		string sKey(key);
		string sSource(IS_EMPTY(source)?"":source);
		return _string2LDIF(sKey, sSource);
	}
	string _string2LDIF(string& key, string& source);
	string _string2LDIF(const char* key, string& source) {
		if (IS_EMPTY(key)) {
			return "";
		}
		string val(key);
		return _string2LDIF(val, source);
	}


	// Donn�es membres priv�es
	//
private:
	charUtils					encoder_;			// Gestion de l'encodage des caract�res

	LDIFUserDatas				attributesToSave_;	// Attributs � sauvegarder

	// Dans le fichier XML ...
	string						usersOU_;			// OU pour les comptes
	string						shortUsersOU_;		// Nom court
	string						baseDN_;

	LDIFUserDatas				add2All_;			// Attributs ajout�s � tous les objets
	deque<string>				mandatories_;		// Attributs obligatoires
	LDIFUserDatas				exclusions_;		// Valeurs d'attribut � ne pas copier

	bool						newFile_;
};

#endif // #ifndef __LDAP_2_FILE_LDIF_OUTPUT_FILE_h__

// EOF
