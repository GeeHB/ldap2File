//---------------------------------------------------------------------------
//--
//--	FICHIER	: XMLParser.cpp
//--
//--	AUTEUR	: Jérôme Henry-Barnaudière - JHB
//--
//--	PROJET	: ldap2File
//--
//--    COMPATIBILITE : Win32 | Linux  Fedora (34 et +) / CentOS (7 & 8)
//--
//---------------------------------------------------------------------------
//--
//--	DESCRIPTION:
//--
//--		Implémentation de la classe XMLParser pour la lecture
//--		des fichiers de paramètres et/ou de configuration au format XML
//--
//---------------------------------------------------------------------------
//--
//--	MODIFICATIONS:
//--	-------------
//--
//--	28/11/2016 - JHB - Création
//--
//--	17/06/2022 - JHB - Version 22.6.4
//--
//---------------------------------------------------------------------------

#include "XMLParser.h"

//
// Implémentation de la classe
//

// Construction
//
XMLParser::XMLParser(const char* rootName, folders* pFolders, logs* pLogs, bool loadComments)
{
	if (IS_EMPTY(rootName)) {
		throw LDAPException("XMLParser - Pas de nom pour la racine", RET_TYPE::RET_INCOMPLETE_FILE);
	}

	// Initialisation des paramètres
	baseRootName_ = rootName;
	folders_ = pFolders;
	fileName_ = "";
	loadComments_ = loadComments;

#ifdef _WIN32
	encoder_.sourceFormat(charUtils::SOURCE_FORMAT::ISO_8859_15);
#endif // #ifdef _WIN32

	// Type de système de fichier "local"
	//
	defType_ = DEST_TYPE::DEST_FS;		// Par défaut un fichier
	expectedOS_ = CURRENT_OS;

	// Traces & logs
	logs_ = pLogs;
	lastError_ = RET_TYPE::RET_OK;      // Par d'erreur (pour l'instant)
}

// Vérification de la version
//
void XMLParser::checkProtocol(const string& parametersNode)
{
	valid_ = false;		// On part du principe que ça va coincer ...

	pugi::xml_node node = xmlDocument_.child(baseRootName_.c_str());
	if (0 == parametersNode.length() || IS_EMPTY(node.name())) {
		string msg("Le fichier '");
		msg += fileName_;
		msg += "' n'est pas dans le bon format";
		throw LDAPException(msg, RET_TYPE::RET_INVALID_FILE);
	}

	// Version du protocole
	string currentVersion(node.attribute(XML_ROOT_VERSION_ATTR).value());
	if (XML_CONF_VERSION_VAL != currentVersion) {
		// Pas la "bonne" version
		string msg("La version du schema XML dans le fichier '");
		msg += fileName_;
		msg += "' n'est pas correcte. Version du fichier : ";
		msg += currentVersion;
		msg += " - Version attendue : ";
		msg += XML_CONF_VERSION_VAL;
		throw LDAPException(msg, RET_TYPE::RET_INVALID_XML_VERSION);
	}

	// On se positionne à la "racine" des paramètres
	paramsRoot_ = node.child(parametersNode.c_str());
	if (IS_EMPTY(paramsRoot_.name())) {
		// Pas le bon document
		string msg("Pas de noeud");
		msg += parametersNode;
		msg += " dans le fichier '";
		msg += fileName_;
		msg += "'";;
		throw LDAPException(msg, RET_TYPE::RET_INCOMPLETE_FILE);
	}

	// Si je suis arrivé ici c'est que tout est correct !
	valid_ = true;
}

// Enregistrement du fichier
//
bool XMLParser::save()
{
	try {
		return xmlDocument_.save_file(fileName_.c_str(), PUGIXML_TEXT("\t"), pugi::format_default | pugi::format_save_file_text, pugi::encoding_utf8);
	}
	catch (...) {
		return false;
	}

	// Ok (inutile)
	return true;
}

// Recherche d'un noeud "fils" ayant une valeur d'attribut particulière
//
pugi::xml_node XMLParser::findChildNode(const pugi::xml_node& parent, const string& childName, const string& attrName, const string& attrValue, bool searchDefValue)
{
	// On cherche la "bonne" valeur
	pugi::xml_node found = _findChildNode(parent, childName, attrName, attrValue);

	// Trouvé ?
	if (!IS_EMPTY(found.name())){
		return found;
	}

	if (false == searchDefValue) {
		// On se contente de cette valeur
		return found;
	}

	// Sinon on cherche la valeur sans l'attribut
	string empty("");
	return _findChildNode(parent, childName, attrName, empty);
}

// Méthode "sous" findChildNode
pugi::xml_node XMLParser::_findChildNode(const pugi::xml_node& parent, const string& childName, const string& attrName, const string& attrValue)
{
	// Recherche du "noeud"
	bool found(false);
	pugi::xml_node childNode = parent.child(childName.c_str());
	pugi::xml_attribute attr;
	while (!IS_EMPTY(childNode.name()) && !found) {
		attr = childNode.attribute(attrName.c_str());

		// Je cherche une valeur d'attribut (ou rien auquel le premier vide fera l'affaire) ?
		found = (attrValue.length() ? (attrValue == attr.value()) : IS_EMPTY(attr.value()));

		// Noeud suivant
		if (!found) {
			childNode = childNode.next_sibling(childName.c_str());
		}
	}

	// On retourne le noeud si trouvé ou un noeud vide
	return childNode;
}

// Utilitaires
//
DEST_TYPE XMLParser::string2FolderType(const char* str)
{
	if (!IS_EMPTY(str)) {
		string upper("");
		upper = charUtils::strupr(str);

		if (0 == charUtils::stricmp(TYPE_DEST_FS, upper.c_str())) {
			return DEST_TYPE::DEST_FILE_SYSTEM;	// = DEST_FS
		}
		else {
			if (0 == charUtils::stricmp(TYPE_DEST_FTP, upper.c_str())) {
				return DEST_TYPE::DEST_FTP;
			}
			else {
				if (0 == charUtils::stricmp(TYPE_DEST_EMAIL, upper.c_str())) {
					return DEST_TYPE::DEST_EMAIL;
				}
				else {
					if (0 == charUtils::stricmp(TYPE_DEST_SCP, upper.c_str())) {
						return DEST_TYPE::DEST_SCP;
					}
				}
			}
		}
	}

	// Autre ...
	return DEST_TYPE::DEST_UNKNOWN;
}

// Conversions
//
unsigned int XMLParser::_value2LinkType(string& value)
{
	if (value == XML_LINK_EMAIL){
		return DATA_LINK_EMAIL;
	}
	else{
		if (value == XML_LINK_HTTP){
			return DATA_LINK_HTTP;
		}
		else{
			if (value == XML_LINK_IMAGE){
				return DATA_LINK_IMAGE;
			}
		}
	}

	// Par défaut ...
	return DATA_LINK_NONE;
}

// Chargement du fichier XML
//
bool XMLParser::_load()
{
	// Le fichier doit exister et être non-vide
	if (0 == fileName_.size()) {
	    string message("Pas de nom pour le fichier XML");
		if (logs_) {
		    logs_->add(logs::TRACE_TYPE::ERR, message.c_str());
        }

		throw LDAPException(message, RET_TYPE::RET_INCOMPLETE_FILE);

		// ???
		return false;
	}

	if (!sFileSystem::exists(fileName_) ||
		0 == sFileSystem::file_size(fileName_)) {

		string message("Le fichier '");
        message += fileName_;
        message += " n'existe pas ou est vide";

		/*
		if (logs_) {
            logs_->add(logs::TRACE_TYPE::ERR, message.c_str());
        }
        */

        throw LDAPException(message, RET_TYPE::RET_INCOMPLETE_FILE);

        // !!!
		return false;
	}

	// Tentative d'ouverture du fichier
	int options = (loadComments_? pugi::parse_default | pugi::parse_comments : pugi::parse_default);
	string erreur("");
	pugi::xml_parse_result result;

	try {
		result = xmlDocument_.load_file(fileName(), options);
	}
	catch (...) {
		if (logs_) {
            logs_->add(logs::TRACE_TYPE::ERR, "Erreur inconnue lors du chargement de '%s'", fileName_.c_str());
        }
		return false;
	}

	// Analyse du code retour
	//
	switch (result.status) {
	case pugi::status_ok:
		return true;

	case pugi::status_file_not_found:
		erreur = "Le fichier n'existe pas : '";
		break;

	case pugi::status_io_error:
		erreur = "Erreur(s) lors de la lecture de '";
		break;

	case pugi::status_out_of_memory:
		erreur = "Erreur mémoire pendant la lecture de '";
		break;

	case pugi::status_unrecognized_tag:
		erreur = "Tag invalide au caractère ";
		erreur += charUtils::itoa(result.offset);
		erreur += " dans '";
		break;

	case pugi::status_bad_comment:
	case pugi::status_bad_cdata:
	case pugi::status_bad_doctype:
	case pugi::status_bad_pcdata:
		erreur = "Format invalide au caractère ";
		erreur += charUtils::itoa(result.offset);
		erreur += " dans '";
		break;

	case pugi::status_bad_start_element:
		erreur = "Début d'élément invalide au caractère ";
		erreur += charUtils::itoa(result.offset);
		erreur += " dans '";
		break;

	case pugi::status_bad_attribute:
		erreur = "Erreur d'attribut au caractère ";
		erreur += charUtils::itoa(result.offset);
		erreur += " dans '";
		break;

	case pugi::status_bad_end_element:
		erreur = "Fin d'élément invalide au caractère ";
		erreur += charUtils::itoa(result.offset);
		erreur += " dans '";
		break;

	case pugi::status_end_element_mismatch:
		erreur = "La fermeture d'élement ne correspond à aucun élément ouvert au caractère ";
		erreur += charUtils::itoa(result.offset);
		erreur += " dans '";
		break;

	case pugi::status_no_document_element:
		erreur = "Document invalide ou vide : '";
		break;

	case pugi::status_internal_error:
	default:
		erreur = "Erreur inconnue lors du chargement de '";
		break;
	}

	erreur += fileName_;
	erreur += "'";

	if (logs_) {
        logs_->add(logs::TRACE_TYPE::ERR, erreur.c_str());
    }

	return false;
}

// EOF
