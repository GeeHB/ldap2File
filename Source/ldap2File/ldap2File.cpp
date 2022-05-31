//---------------------------------------------------------------------------
//--
//--	FICHIER	: ldap2File.cpp
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
//--		Fichier principal
//--
//---------------------------------------------------------------------------
//--
//--	APPELS :
//--
//--		[fichiers ou dossier]
//--		-base:{folder} : {folder} est la racine de l'application
//--		-s - Mode silencieux
//--		-c - Effacement des fichiers après traitement
//--		-f:{min} - Annalyse régulière (si pas de suppression) toutes les {min} minutes
//--		-d:{directory} - Annalyse de tous les fichiers contenus dans {directory}
//--
//--		+ Analyse répétitive d'un dossier : -d:C:\ldapTools\web -f:15 -s
//--
//--		+ Un ou plusieurs fichier(s) : C:\ldapTools\web\SMH-HTML.xml [fichiers]
//--
//--		+ Avec suppression du fichier : C:\ldapTools\web\SMH-HTML.xml -c
//--
//---------------------------------------------------------------------------
//--
//--	MODIFICATIONS:
//--	-------------
//--
//--	17/12/2015 - JHB - Création
//--
//--	06/05/2022 - JHB - Version 22.5.1
//--
//---------------------------------------------------------------------------

#include "sharedConsts.h"
#include "sFileSystem.h"
#include "LDAPBrowser.h"

#include "XMLParser.h"

#ifndef _WIN32
#include <dirent.h>		// Gestion des repertoires
#include <ctime>
#endif // _WIN32

//
// Prototypes
//
bool _checkCurrentVersion(string& error);
bool _getFolderContent(const string& source, list<string>& content, logs* pLogs);
int _getTickCount();
void _now();
bool _updateConfigurationFile(string path);
void _usage();

//
// Fonctions
//

// Point d'entrée du programme
//
int main(int argc, const char* argv[]) {

#ifdef _WIN32
	// On bascule la console en UTF8 !!!
	// tout au moins on essaye ...
	SetConsoleOutputCP(CP_UTF8);
	setvbuf(stdout, nullptr, _IOFBF, 1000);
#endif // _WIN32

	// Nom complet de l'application
	//
	string fullAppName(argv[0]);

	// Dossier de l'application
    //
    string appPath("");
	string binName(sFileSystem::split(fullAppName, appPath));
    if (0 == appPath.length() || "." == appPath) {
        appPath = sFileSystem::current_path();
    }

	// Binaire et sa version
	//
	cout << binName << " - Version " << APP_RELEASE << " pour " << CURRENT_OS;
#ifdef _DEBUG
	cout << " - DEBUG";
#endif // |DEBUG
	cout << endl;
	cout << "Copyright © " << APP_COPYRIGHT << endl;

	// Paramètres de la ligne de commandes
	//
	if (argc < 2) {
		_usage();
		return 1;
	}

#ifdef _WIN32
	bool verbose(true);
#endif // _WIN32

	// Vérification de la ligne de commandes
	//
	int retCode(0);
	folders myFolders;		// Liste des dossiers utilisés par l'application
	logs myLogs;
	confFile configurationFile(&myFolders, &myLogs);
	string file("");

	try {
		// Dossier "de base" de l'application
		//
		string folder("");

		// Passé en ligne de commande ?
		for (int index = 1; 0 == folder.size() && index < argc; index++) {
			if (argv[index] == strstr(argv[index], CMD_LINE_BASE_FOLDER)) {
				folder = argv[index] + strlen(CMD_LINE_BASE_FOLDER);
			}
		}

		if (0 == folder.size()) {
/*#ifdef _DEBUG
			folder = FOLDER_APP_DEFAULT;
#else*/
			folder = appPath;
//#endif // _DEBUG
		}

		// Quelques vérifications
		//
		string error("");
        if (false == _checkCurrentVersion(error)) {
            if (error.length()) {
                cout << error;
                return 1;
            }

            // Mise à jour du fichier de configuration
            if (false == _updateConfigurationFile(folder)) {
                return 1;
            }
        }

		// Ajout du dossier de l'application ...
		myFolders.add(folders::FOLDER_TYPE::FOLDER_APP, folder);
		myFolders.add(folders::FOLDER_TYPE::FOLDER_LOGS, STR_FOLDER_LOGS);				// sous dossier des logs
		myFolders.add(folders::FOLDER_TYPE::FOLDER_TEMPLATES, STR_FOLDER_TEMPLATES);	// sous dossier des modèles
		myFolders.add(folders::FOLDER_TYPE::FOLDER_TEMP, STR_FOLDER_TEMP);				// fichiers temporaires
		myFolders.add(folders::FOLDER_TYPE::FOLDER_OUTPUTS, STR_FOLDER_OUTPUTS);		// pour les fichiers générés

		// Ouverture du fichier de configuration
        //
        file = sFileSystem::merge(folder, XML_CONF_FILE);
        if (!configurationFile.open(file.c_str())) {
            // Une "petite" erreur ...
            return 1;
        }
	}
	catch (LDAPException& e) {
		cout << "Erreur : " << e.what() << endl;
		return 1;
	}
	catch (...) {
		// Erreur inconnue
		cout << "Erreur inconnue lors de l'initialisation" << endl;
		return 1;
	}

	try{
		// Informations sur les logs ...
		LOGINFOS lInfos;
		configurationFile.logInfos(lInfos);

		if (lInfos.folder_.size()) {
			// Mise à jour de dossier de logs
			myFolders.add(folders::FOLDER_TYPE::FOLDER_LOGS, lInfos.folder_);
		}

		// Le dossier des logs doit exister (on a auparavant tenté de le créer s'il n'existait pas)
		folders::folder* logFolder = myFolders.find(folders::FOLDER_TYPE::FOLDER_LOGS);
		if (NULL == logFolder) {
			throw LDAPException("Le dossier des logs n'a pu être ouvert ou crée", RET_TYPE::RET_ACCESS_ERROR);
		}

		cout << "Programme : " << fullAppName << endl;
		cout << "Dossiers de l'application : " << endl;
		cout << "\t - racine : " << myFolders.find(folders::FOLDER_TYPE::FOLDER_APP)->path() << endl;
		cout << "\t - logs : " << myFolders.find(folders::FOLDER_TYPE::FOLDER_LOGS)->path() << endl;
		cout << "\t - modèles : " << myFolders.find(folders::FOLDER_TYPE::FOLDER_TEMPLATES)->path() << endl;
		cout << "\t - temporaires : " << myFolders.find(folders::FOLDER_TYPE::FOLDER_TEMP)->path() << endl;

		// Initialisation du fichier de logs
		//
		myLogs.init(lInfos.mode_.c_str(), logFolder->path(), lInfos.fileName_.c_str());
		myLogs.setFileAge(lInfos.duration_);	// JHB -> retrouver le corps de la méthode !!!

		myLogs.add(logs::TRACE_TYPE::LOG, "----------------------------------------------------------------------------------------------------------------------------");
		string copyRight("---- %s - version %s pour %s");
#ifdef _DEBUG
		copyRight += " -- DEBUG";
#endif // _DEBUG
		copyRight += " - %s";
		myLogs.add(logs::TRACE_TYPE::LOG, copyRight.c_str(), APP_SHORT_NAME, APP_RELEASE, CURRENT_OS, APP_DESC);
		myLogs.add(logs::TRACE_TYPE::LOG, "---- Copyright %s", APP_COPYRIGHT);
		myLogs.add(logs::TRACE_TYPE::LOG, "Lancement de l'application");

		if (LOG_DURATION_INFINITE != lInfos.duration_) {
			myLogs.add(logs::TRACE_TYPE::NORMAL, "Conservation des logs %d jours", lInfos.duration_);
		}

		myLogs.add(logs::TRACE_TYPE::LOG, "Binaire : %s", fullAppName.c_str());
		myLogs.add(logs::TRACE_TYPE::LOG, "Fichier de configuration : %s", file.c_str());
		myLogs.add(logs::TRACE_TYPE::LOG, "Dossiers de l'application : ");
		myLogs.add(logs::TRACE_TYPE::LOG, "\t- app : %s", myFolders.find(folders::FOLDER_TYPE::FOLDER_APP)->path());
		myLogs.add(logs::TRACE_TYPE::LOG, "\t- fichiers : %s", myFolders.find(folders::FOLDER_TYPE::FOLDER_OUTPUTS)->path());
		myLogs.add(logs::TRACE_TYPE::LOG, "\t- templates : %s", myFolders.find(folders::FOLDER_TYPE::FOLDER_TEMPLATES)->path());
		myLogs.add(logs::TRACE_TYPE::LOG, "\t- temporaires : %s", myFolders.find(folders::FOLDER_TYPE::FOLDER_TEMP)->path());
	}
	catch (LDAPException& e) {
		cout << "Erreur : " << e.what() << endl;
		myLogs.add(logs::TRACE_TYPE::ERR, "Erreur : %s", e.what());
		retCode = 1;
	}
	catch (...) {
		// Erreur inconnue
		myLogs.add(logs::TRACE_TYPE::ERR, "Erreur inconnue");
		retCode = 1;
	}

	// Le logs existent, on peut maintenant
	// faire un peu de c++ ...
	struct _finalLogs {
		logs logs_;
		_finalLogs(const logs& other) {
			logs_ = other;
		}
		virtual ~_finalLogs() {
			logs_.add(logs::TRACE_TYPE::LOG, "Fermeture de l'application");
			logs_.add(logs::TRACE_TYPE::LOG, "----------------------------------------------------------------------------------------------------------------------------");
		}
	};

	_finalLogs myTrace(myLogs);

	list<string> files;
	size_t filesGenerated(0);

	try {
		int freq(0);
		bool removeFile(false);
		string remoteFolder("");

		//
		// Les fichiers à analyser sont soit passés en ligne de commande soit contenus dans un dossier
		//

		// Récupération de tous les noms de fichier en ligne de commande
		//
		for (int index = 1; index < argc; index++) {
#ifdef _WIN32
			if (argv[index] == strstr(argv[index], CMD_NO_VERBOSE)) {
				// Pas d'affichages
				verbose = false;
			}
			else {
#endif // _WIN32
				if (argv[index] == strstr(argv[index], CMD_REMOVE_FILE)) {
					// Suppression du ou des fichier(s)
					removeFile = true;
				}
				else {
					if (argv[index] == strstr(argv[index], CMD_LINE_FREQ)) {
						// Analyse régulière
						freq = atoi(argv[index] + strlen(CMD_LINE_FREQ)) * 60000;
					}
					else {
						if (argv[index] == strstr(argv[index], CMD_LINE_DIR)) {
							// Analyse d'un dossier complet
							remoteFolder = argv[index] + strlen(CMD_LINE_DIR);
						}
						else {
							if (argv[index] == strstr(argv[index], CMD_LINE_BASE_FOLDER)) {
								// Déja lu ...
							}
							else {
								// Un fichier ...
								files.push_back(sFileSystem::complete(argv[index]));
							}
						}
					}
				}
			}
#ifdef _WIN32
		}
#endif // _WIN32

		// Analyse d'un dossier
		if (remoteFolder.size()) {
			if (false == _getFolderContent(remoteFolder, files, &myLogs)){
			    return 1;
			}
		}
		else {
			// Si un seul fichier, c'est peut être un dossier ...
			if (1 == files.size() && sFileSystem::is_directory(*files.begin())) {
					remoteFolder = (*files.begin());
					_getFolderContent(remoteFolder, files, &myLogs);
			}
		}

		bool done(0 == files.size());

		// Analyse des fichiers de commandes
		//
		int currentLaunchTime(0);
		LDAPBrowser requester(&myLogs, &configurationFile);
		RET_TYPE retType(RET_TYPE::RET_INVALID_FILE);
		std::string shortName("");
		while (!done && 0 == retCode) {
			currentLaunchTime = _getTickCount();

			for (list<string>::iterator it = files.begin(); 0 == retCode && it != files.end(); it++) {

                _now();

				// Affichage du nom court du fichier
				cout << sFileSystem::split((*it));
				cout.flush();

				// Par défaut le fichier n'est pas bon ...
				retType = RET_TYPE::RET_INVALID_FILE;

				// Génération du fichier en question
				if (configurationFile.openCommandFile((*it).c_str(), retType) &&
					RET_TYPE::RET_OK == (retType = requester.browse())) {
					cout << " - [ok]";
					filesGenerated++;
				}
				else {
					cout << " - [ko] - ";
					switch (retType) {
					case RET_TYPE::RET_ACCESS_ERROR:
                        cout << "Erreur d'accès";
					    break;

                    case RET_TYPE::RET_ALLOCATION_ERROR:
                        cout << "Erreur d'allocation mémoire";
					    break;

					case RET_TYPE::RET_INVALID_FILE:
					    cout << "Le fichier n'existe pas ou est vide";
					    break;

                    case RET_TYPE::RET_INCOMPLETE_FILE:
                        cout << "Le fichier (conf ou commande) n'est pas complet";
					    break;

                    case RET_TYPE::RET_INVALID_XML_VERSION:
					    cout << "Version XML incorrecte dans les fichiers";
					    break;

					case RET_TYPE::RET_INVALID_PARAMETERS:
						cout << "Paramètres invalides";
						break;

					case RET_TYPE::RET_ERROR_NO_DESTINATION:
						cout << "Pas de destination valide";
						break;

					case RET_TYPE::RET_INVALID_OUTPUT_FORMAT:
						cout << "Format de fichier de sortie inconnu";
						break;

					case RET_TYPE::RET_FILE_TO_DELETE:
						cout << "La date limite est dépassée - Le fichier doit être supprimé";
						removeFile = true;
						break;

					case RET_TYPE::RET_NON_BLOCKING_ERROR:
						cout << "Erreur(s) non bloquante(s)";
						filesGenerated++;	// L'erreur n'a pas empêchée la génération du fichier
						break;

					case RET_TYPE::RET_LDAP_ERROR:
						cout << "Erreur LDAP";
						retCode = 1;            // L'erreur est bloquante (pas de connexion au serveur ...)
						break;

					case RET_TYPE::RET_UNABLE_TO_SAVE:
						cout << "Erreur de sauvegarde de fichier";
						break;

					case RET_TYPE::RET_NO_SUCH_CONTAINER_ERROR:
						cout << "Critère de recherche invalide";
						break;

					case RET_TYPE::RET_ERROR_NO_CONTAINER:
						cout << "Aucun container trouvé dans l'Annuaire";
						break;

					case RET_TYPE::RET_BLOCKING_ERROR:
					default:
						cout << "Erreur(s) bloquante(s)";
						retCode = 1;
						break;
					} // switch
				} // else

				cout << endl;

				// Suppression du fichier de commandes
				if (removeFile) {
#ifndef _DEBUG
					sFileSystem::remove((*(files.begin())).c_str());
#endif // #ifndef _DEBUG

					// Il ne peut pas y avoir d'analyse régulière ...
					freq = 0;
				}
			} // for

			if (freq) {
				// On attend un peu
				_now();
				cout << "Sleep " << (freq / 60000) << " min" << endl;
				myLogs.add(logs::TRACE_TYPE::LOG, "Sleep %d min.", freq / 60000);

				int duration = _getTickCount() - currentLaunchTime;	// Durée des traitements précédents
				duration = (duration > freq ? freq : freq - duration);
#ifdef _WIN32
				Sleep(duration);
#else
				sleep(duration);
#endif //#ifdef _WIN32

				// Analyse du dossier ?
				if (remoteFolder.size()) {
					_getFolderContent(remoteFolder, files, &myLogs);
					done = (0 == files.size());
				}
			}
			else {
				done = true;
			}
		}
	}
	catch (LDAPException& e) {
		// Une ereur bloquante
		myLogs.add(logs::TRACE_TYPE::ERR, e.what());
		cout << "Erreur bloquante. Arrêt des traitements" << endl;
		retCode = 1;
	}
	catch (...) {
		// Erreur inconnue
		myLogs.add(logs::TRACE_TYPE::ERR, "Erreur inconnue");
		retCode = 1;
	}


#ifdef _WIN32
	if (verbose) {
		char message[200];
		sprintf_s(message, 199, "%d fichier(s) genere(s)", filesGenerated);

		MessageBox(NULL, message, APP_SHORT_NAME, MB_ICONINFORMATION);
	}
#else
	cout << filesGenerated << " fichier" << (filesGenerated > 1 ? "s " : " ") << "crée" << (filesGenerated > 1 ? "s" : " ") << endl;
#endif // _WIN32

	// Fin
	myLogs.add(logs::TRACE_TYPE::LOG, "%d / %d fichier(s) crée(s)", filesGenerated, files.size());

	return retCode;
}

// Lecture du contenu d'un dossier
//
bool _getFolderContent(const string& srcPath, list<string>& content, logs* pLogs)
{
	if (!srcPath.size()) {
		return false;
	}

	// Remplacement ?
	std::string source(sFileSystem::complete(srcPath));

	// Le dossier doit exister
	if (!sFileSystem::exists(source)) {
		cout << "Le dossier '" << source.c_str() << "' n'existe pas" << endl;

		if (pLogs) {
			pLogs->add(logs::TRACE_TYPE::ERR, "Le dossier '%s' n'existe pas", source.c_str());
		}

		return false;
	}

	cout << "Parcours du dossier '" << source.c_str() << "'" << endl;
	if (pLogs) {
		pLogs->add(logs::TRACE_TYPE::LOG, "Parcours du dossier '%s'", source.c_str());
	}

	// La liste est vide
	content.clear();

	// Parcours du dossier
#ifdef _WIN32
	WIN32_FIND_DATA wfd;
	string sDir(source);
	HANDLE hFind(INVALID_HANDLE_VALUE);
	DWORD dwError(0);

	sDir += "\\*.*";
	if (INVALID_HANDLE_VALUE != (hFind = FindFirstFile(sDir.c_str(), &wfd))) {
		do {
			if ((wfd.dwFileAttributes == FILE_ATTRIBUTE_NORMAL) || (wfd.dwFileAttributes & FILE_ATTRIBUTE_READONLY)
				|| (wfd.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE)) {
				sDir = source;
				sDir += "\\";
				sDir += wfd.cFileName;
				content.push_back(sDir);
			}
		} while (FindNextFile(hFind, &wfd) != 0);

		FindClose(hFind);
	}
#else
	DIR* d;
	struct dirent *dir;
	string fullName;
	d = opendir(source.c_str());
	if (d) {
		while ((dir = readdir(d)) != NULL) {
			if (dir->d_type == DT_REG) {
				fullName = source;
				fullName += "/";
				fullName += dir->d_name;
				content.push_back(fullName);
			}
		}

		closedir(d);
	}
#endif // _WIN32

	cout << content.size() << " fichier(s) à analyser" << endl;
	if (pLogs) {
		pLogs->add(logs::TRACE_TYPE::LOG, "%d fichier(s) à analyser", content.size());
	}

	return true;
}

// Heure locale
//
void _now()
{
	time_t now = time(0);
	tm *ltm = localtime(&now);

	cout << std::setfill('0') << std::setw(2) << ltm->tm_hour << ":" << std::setfill('0') << std::setw(2) << ltm->tm_min << ":" << std::setfill('0') << std::setw(2) << ltm->tm_sec << " - ";
}

// Durée depuis le lancement de l'OS
//
int _getTickCount()
{
#ifdef _WIN32
	return GetTickCount();
#else
	struct timespec ts;
	unsigned theTick = 0U;
	clock_gettime(CLOCK_REALTIME, &ts);
	theTick = ts.tv_nsec / 1000000;
	theTick += ts.tv_sec * 1000;
	return theTick;
#endif // _WIN32
}

// Vérification de la version courante de l'application et mise à jour
//
//	retourne un booléen indiquant si l'application est à jour
//
bool _checkCurrentVersion(string& error)
{
	bool valid(false);
	error = "";

#ifdef _WIN32
	CRegEntry entry;
	if (FALSE == entry.SetBaseDirectory(REG_LDAP2FILE_ROOT, REG_LDAP2FILE_PATH)) {
		error = "Erreur - Impossible d'accdéder à la base de regsitres";
		return false;	// pas à jour
	}

	string version = entry.GetPrivateString(REG_LDAP2FILE_SECTION, REG_LDAP2FILE_VER_KEY, REG_LDAP2FILE_VER_DEF);

	if (0 == version.length()) {
		cout << "Premier lancement de l'application" << endl;
	}

	if (APP_RELEASE != version) {
		// Mise à jour de la version
#ifndef _DEBUG
		entry.WritePrivateString(REG_LDAP2FILE_SECTION, REG_LDAP2FILE_VER_KEY, APP_RELEASE);
#endif // _DEBUG
	}
	else {
		valid = true;
	}
#endif // _WIN32

	// Ok ?
	return valid;
}

// Mise à jour du fichier de configuration
//
bool _updateConfigurationFile(string path)
{
	// Fichier de configuration
	string confFile = sFileSystem::merge(path, XML_CONF_FILE);
	XMLParser xmlConf(confFile.c_str(), XML_ROOT_LDAPTOOLS_NODE, NULL, NULL, true);

	try {
		// Chargement du fichier
		xmlConf.load();

		// Vérifications
		xmlConf.checkProtocol(XML_CONF_NODE);
	}
	catch (LDAPException& e) {
		cout << e.what() << endl;
		return false;
	}

	// Ma "racine"
	pugi::xml_node paramsRoot = (*xmlConf.paramsRoot());

	// Dossier de l'application
	//
	bool update(false);
	pugi::xml_node childNode = xmlConf.findChildNode(paramsRoot, XML_CONF_FOLDER_NODE, XML_CONF_FOLDER_OS_ATTR, xmlConf.expectedOS(), false);

	// Trouvé ?
	if (!IS_EMPTY(childNode.name())) {
		string val = childNode.first_child().value();
#ifdef _DEBUG
		cout << "Dossier de l'application : " << val << endl;
#endif // _DEBUG

		// Le dossier doit exister !
		if (!val.length() || !sFileSystem::exists(val)) {

#ifdef _DEBUG
			cout << "\t- Le dossier n'existe pas,  mise à jour avec '" << path << "'" << endl;
#endif // _DEBUG

			// Mise à jour
			childNode.first_child().set_value(path.c_str());
			update = true;
		}
	}
	else {
		// Pas de dossier => on ajoute la version par défaut
#ifdef _DEBUG
		cout << "Pas de dossier pour l'application,  mise à jour avec " << path << endl;
#endif // _DEBUG

		childNode = paramsRoot.prepend_child(XML_CONF_FOLDER_NODE);
		childNode.append_child(pugi::node_pcdata).set_value(path.c_str());
		childNode.append_attribute(XML_CONF_FOLDER_OS_ATTR) = xmlConf.expectedOS();
		update = true;
	}

	// Dossier des logs
	//
	string baseFolder(path);
	path = sFileSystem::merge(path, STR_FOLDER_LOGS);
	pugi::xml_node logNode = paramsRoot.child(XML_CONF_LOGS_NODE);
	if (IS_EMPTY(logNode.name())) {
		cout << "Erreur - pas de noeud " << XML_CONF_LOGS_NODE << " dans le fichier '" << confFile << "'" << endl;
		return false;
	}

	childNode = xmlConf.findChildNode(logNode, XML_CONF_LOGS_FOLDER_NODE, XML_CONF_FOLDER_OS_ATTR, xmlConf.expectedOS(), true);

	// Trouvé ?
	//
	if (!IS_EMPTY(childNode.name())) {
		string val = childNode.first_child().value();

#ifdef _DEBUG
		cout << "Dossier des logs : " << val << endl;
#endif // _DEBUG

		if (folders::isSubFolder(val)) {
			// C'est un nom court
			val = sFileSystem::merge(baseFolder, val);
		}

		if (!sFileSystem::exists(val) && !sFileSystem::create_directory(val)) {
#ifdef _DEBUG
			cout << "\t- Le dossier n'existe pas - Utilisation du dossier '" << path << "'" << endl;
#endif // _DEBUG

			// Mise à jour
			childNode.first_child().set_value(path.c_str());
			update = true;
		}
#ifdef _DEBUG
		else {
			cout << "Logs : Utilisation du dossier '" << val << "'" << endl;
		}
	}
	else {
		// Il n'existe pas => rien à faire
		cout << "\t- Pas de dossier précisé pour les logs => rien à faire" << endl;
	}
#else
}
#endif // _DEBUG

	//
	// Mise à jour du fichier
	//
	if (update) {

		// Conservation du fichier (s'il n'existe pas déja)
		string destFile(confFile);
		destFile += ".old";
		if (!sFileSystem::exists(destFile)) {
			sFileSystem::copy_file(confFile, destFile);
		}

		// Sauvegarde
		if (false == xmlConf.save()) {
			cout << "Erreur lors de la sauvegarde du fichier '" << confFile << "'" << endl;
			return false;
		}
	}

	// Ok
	return true;
}

// Usage (paramètres) de la'application
//
void _usage()
{
	cout << "\n" << APP_FULL_NAME << " {files or folder} [-base:{folder}] [-d:{folder}] [-f:{freq}] [-o:{output filename}] [-c] [-s]" << endl;
	cout << "\n\t {files or folder} : Listes des fichiers de commande à traiter. Lorsqu'un seul nom est fourni, et qu'il s'agit d'un dossier, tout le contenu du dossier sera traité (identique à -d:{folder})" << endl;
	cout << "\n\t -base:{folder} : Le dossier 'folder' est considéré comme le dossier de l'application. Par défaut, le dossier de l'application est celui dans lequel se trouve le binaire." << endl;
	cout << "\n\t -d:{folder} : Analyse de tous les fichiers de commandes contenus dans le dossier {folder}" << endl;
	cout << "\n\t -f:{freq} : Analyse périodique des fichiers et des dossiers. Freq est la fréquence d'analyse en minutes" << endl;
	cout << "\n\t -o:{output-file} : Le fichier généré sera renommé en {output-file] même si le fichier de commande indique une autre destination" << endl;
	cout << "\n\t -c : Suppression du fichier de commande après traitements" << endl;
	cout << "\n\t -s : Windows uniquement = pas de MessageBox" << endl;

	cout << "\nExemples d'appels:" << endl;

	cout << "\n- Analyse et traitement des fichiers de commande du dossier c:\\my datas\\test toutes les 30 minutes:" << endl;
	cout << "\n\t" << APP_FULL_NAME << " -d:\"c:\\my datas\\test\" -f:30" << endl;

	cout << "\n- Traitement du fichier '~/ldap2Files/datas/dsun.xml'; le dossier de l'application étant '/shared/cmdFiles" << endl;
	cout << "\n\t" << APP_FULL_NAME << " -base /shared/cmdFiles ~/ldap2Files/datas/dsun.xml" << endl;

	cout << "\n- Traitement des 5 fichiers passes en ligne de commande ! " << endl;
	cout << "\n\t" << APP_FULL_NAME << " file1.xml file2.xml file3.xml file4.xml file5.xml" << endl;
}

// EOF
