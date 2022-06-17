//---------------------------------------------------------------------------
//--
//--	FICHIER	: roles.h
//--
//--	AUTEUR	: Jérôme Henry-Barnaudière - JHB
//--
//--	PROJET	: ldap2File
//--
//--    COMPATIBILITE : Win32 | Linux Fedora (34 et +) / CentOS (7 & 8)
//--
//---------------------------------------------------------------------------
//--
//--	DESCRIPTION:
//--
//--		Définition de la classe roles
//--
//--		Lisre des "rôles" ie. association Nom <-> attributs LDAP dans les requêtes
//--
//---------------------------------------------------------------------------
//--
//--	MODIFICATIONS:
//--	-------------
//--
//--	08/06/2022 - JHB - Création
//--
//--	17/06/2022 - JHB - Version 22.6.4
//--
//---------------------------------------------------------------------------

#ifndef __LDAP_2_ATTRIBUTE_ROLES_h__
#define __LDAP_2_ATTRIBUTE_ROLES_h__ 1

//--------------------------------------------------------------------------
//--
//-- Definition de la classe
//--
//--------------------------------------------------------------------------

class roles
{
	// Méthodes publiques
public:

	// Construction
	roles()
	{ logs_ = nullptr; }

	// Destruction
	virtual ~roles()
	{}

	// Logs
	void setLogs(logs* logs)
	{ logs_ = logs;}

	// Ajout d'un nom
	void operator +=(std::string& name){
        if (name.size()){
            _addEmptyRole(name.c_str());
        }
    }
	void operator +=(const char* name){
        if (!IS_EMPTY(name)){
            _addEmptyRole(name);
        }
    }

	// Ajout d'un couple {nom du rôle, attribut}
	bool add(std::string& name, std::string& colName, std::string& attribute);

	// Accès
	keyValTuple* operator [](const char* name);
	keyValTuple* operator [](std::string& name){
	    return (*this)[name.c_str()];
	}

	// Taille totale
	size_t size()
	{ return roles_.size(); }

	// Eléménets complètements renseignés
	size_t filled();

	// Nettoyage
	void init()
	{}

	// Méthodes privées
protected:
    // Ajout d'un rôle sans valeur
	void _addEmptyRole(const char* name);

	// Données membres privées
protected:

    logs*						logs_;

	// Liste des rôles
	map<string, keyValTuple*>   roles_;
};

#endif // __LDAP_2_ATTRIBUTE_ROLES_h__

// EOF
