//---------------------------------------------------------------------------
//--
//--	FICHIER	: roles.cpp
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
//--		Implémentation de la classe roles
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
//--	07/06/2022 - JHB - Version 22.6.3
//--
//---------------------------------------------------------------------------

#include "sharedTypes.h"
#include "roles.h"

//--------------------------------------------------------------------------
//--
//-- Implémentation
//--
//--------------------------------------------------------------------------


// Ajout d'un couple {nom du rôle, attribut}
//
bool roles::add(std::string& name, std::string& colName, std::string& attribute)
{
    if (0 == name.size() || 0 == colName.size()){
        // Le nom du rôle et celaui de la colonne doivent être renseignés
        return false;
    }

    // Recherche dans la liste
    map<string, keyValTuple*>::iterator pos = roles_.find(name);
    if (roles_.end() != pos) {
        keyValTuple* role = (*pos).second;

        // La strucuture existe ?
        if (role) {
            // => remplacment des valeurs
            (*pos).second->setKey(colName);
            (*pos).second->setValue(attribute);
        }
        else {
            // Création
            (*pos).second = new keyValTuple(colName, attribute);
        }
    }
    else {
        if (logs_) {
            logs_->add(logs::TRACE_TYPE::ERR, "Rôles - '%s' n'est pas un type de rôle reconnu", name.c_str());
        }
    }

    return true;
}

// Nombre de rôles "complets"
//
size_t roles::filled()
{
    size_t count(0);
    keyValTuple* role(nullptr);
    for (map<string, keyValTuple*>::iterator pos = roles_.begin(); pos != roles_.end(); pos++) {
        role = (*pos).second;
        
        // Le rôle est-il complètement renseigné ?
        if (role && role->key().size() && role->value().size()) {
            count++;
        }
    }

    return count;
}

// Accès
//
keyValTuple* roles::operator [](const char* name)
{
    keyValTuple* role(nullptr);

    if (!IS_EMPTY(name)){
        map<string, keyValTuple*>::iterator pos = roles_.find(name);
        if (roles_.end() != pos) {
            role = (*pos).second;
        }
    }

    // Je retourne un pointeur sur le rôle (ou NULL s'il n'est pas dans la liste)
    return role;
}

// La "vraie" méthode d'ajout d'un nom
//
void roles::_addEmptyRole(const char* name)
{
    // name existe, on s'en est assuré en amont !

    // Déja présent ?
    map<string, keyValTuple*>::iterator pos = roles_.find(name);
    if (roles_.end() == pos) {
        // Il n'exste pas => je peux l'ajouter vide
        roles_[name] = nullptr;
    }
}

// EOF