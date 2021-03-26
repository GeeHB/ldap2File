//---------------------------------------------------------------------------
//--
//--	FICHIER	: XMLParser.cpp
//--
//--	AUTEUR	: Jérôme Henry-Barnaudière - JHB
//--
//--	PROJET	: ldap2File
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
//--	26/03/2021 - JHB - Version 21.3.4
//--
//---------------------------------------------------------------------------

#include "XMLParser.h"

//
// Implémentation de la classe
//


// Construction
//
XMLParser::XMLParser(const char* rootName, folders* pFolders, logFile* logs, bool loadComments)
{
	if (IS_EMPTY(rootName)) {
		throw LDAPException("XMLParser - Pas de nom pour la racine");
	}
	
	// Initialisation des paramètres
	baseRootName_ = rootName;
	folders_ = pFolders;
	logs_ = logs;
	fileName_ = "";
	loadComments_ = loadComments;
	
	encoder_.sourceFormat(charUtils::SOURCE_FORMAT::ISO_8859_15);

	// Type de système de fichier "local"
#ifdef _WIN32
	expectedOS_ = OS_WINDOWS;
	defType_ = DEST_TYPE::DEST_FS_WINDOWS;
#else
#ifdef __APPLE__
	expectedOS_ = OS_MACOS;
	defType_ = DEST_TYPE::DEST_FS_MACOS;
#else
	expectedOS_ = OS_LINUX;
	defType_ = DEST_TYPE::DEST_FS_LINUX;
#endif // __APPLE__
#endif // _WIN32
}

// Vérification de la version
//
void XMLParser::checkProtocol(const string& parametersNode)
{
	pugi::xml_node node = xmlDocument_.child(baseRootName_.c_str());
	if (0 == parametersNode.length() || IS_EMPTY(node.name())) {
		string msg("Le fichier '");
		msg += fileName_;
		msg += "' n'est pas dans le bon format";
		throw LDAPException(msg);
	}

	// Version du protocole
	string currentVersion(node.attribute(XML_ROOT_VERSION_ATTR).value());
	if (XML_CONF_VERSION_VAL != currentVersion) {
		// Pas la "bonne" version
		string msg("La version du fichier '");
		msg += fileName_;
		msg += "' n'est pas correcte. Version attendue : ";
		msg += XML_CONF_VERSION_VAL;
		throw LDAPException(msg);
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
		throw LDAPException(msg);
	}
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
pugi::xml_node XMLParser::findChildNode(const pugi::xml_node& parent, const string& childName, const string& attrName, const string& attrValue)
{
	// Recherche du "noeud"
	bool found(false);
	pugi::xml_node childNode = parent.child(childName.c_str());
	while (!IS_EMPTY(childNode.name()) && !found) {
		found = (attrValue == childNode.attribute(attrName.c_str()).value());

		// noeud suivant
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
	if (!IS_EMPTY(str)){
		string upper("");
		upper = charUtils::strupr(str);

		if (0 == charUtils::stricmp(TYPE_DEST_FS_LINUX, upper.c_str())){
			return DEST_TYPE::DEST_FS_LINUX;
		}
		else{
			if (0 == charUtils::stricmp(TYPE_DEST_FS_MACOS, upper.c_str())){
				return DEST_TYPE::DEST_FS_MACOS;
			}
			else{
				if (0 == charUtils::stricmp(TYPE_DEST_FS_WINDOWS, upper.c_str())){
					return DEST_TYPE::DEST_FS_WINDOWS;
				}
				else{
					if (0 == charUtils::stricmp(TYPE_DEST_FTP, upper.c_str())){
						return DEST_TYPE::DEST_FTP;
					}
					else{
						if (0 == charUtils::stricmp(TYPE_DEST_EMAIL, upper.c_str())){
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
void XMLParser::_load()
{
	// Le fichier doit exister et être non-vide
	if (0 == fileName_.size()) {
		throw LDAPException("Pas de nom pour le fichier de configuration");
	}

	if (!sFileSystem::exists(fileName_) ||
		0 == sFileSystem::file_size(fileName_)) {
		string erreur("Le fichier '");
		erreur += fileName_;
		erreur += "' n'existe pas ou est vide";
		throw LDAPException(erreur);
	}

	// Tentative d'ouverture du fichier
	int options = (loadComments_? pugi::parse_default | pugi::parse_comments : pugi::parse_default);
	string erreur("");
	pugi::xml_parse_result result;

	try {
		result = xmlDocument_.load_file(fileName(), options);	
	}
	catch (...) {
		erreur = "Erreur inconnue lors du chargement de '";
		erreur += fileName_;
		erreur += "'";
		throw LDAPException(erreur);
	}

	// Analyse du code retour
	//
	switch (result.status) {
	case pugi::status_ok:
		return;

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
	throw LDAPException(erreur);
}

// EOF