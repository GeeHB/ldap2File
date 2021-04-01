//---------------------------------------------------------------------------
//--
//--	FICHIER	: ldap2File.cpp
//--
//--	AUTEUR	: Jérôme Henry-Barnaudière - JHB
//--
//--	PROJET	: ldap2File
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
//--	01/04/2021 - JHB - Version 21.4.9
//--
//---------------------------------------------------------------------------

#include "sharedConsts.h"
#include "sFileSystem.h"
#include "ldapBrowser.h"

#include "XMLParser.h"

#ifndef _WIN32
#include <dirent.h>		// Gestion des repertoires
#include <ctime>
#endif // _WIN32

//
// Fonctions
//

// Lecture du contenu d'un dossier
//
void _getFolderContent(const string& source, list<string>& content)
{
	if (!source.size()){
		return;
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
	if (INVALID_HANDLE_VALUE != (hFind = FindFirstFile(sDir.c_str(), &wfd))){
		do{
			if ((wfd.dwFileAttributes == FILE_ATTRIBUTE_NORMAL) || (wfd.dwFileAttributes & FILE_ATTRIBUTE_READONLY)
				|| (wfd.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE)){
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
	if (d){
		while ((dir = readdir(d)) != NULL){
			if (dir->d_type == DT_REG){
				fullName = source;
				fullName += "/";
				fullName += dir->d_name;
				content.push_back(fullName);
			}
		}

		closedir(d);
	}
#endif // _WIN32
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

// Mise à jour des fichiers de configuration
//
bool _updateConfigurationFile(const char* appName)
{
	if (IS_EMPTY(appName)) {
		cout << "Erreur - Paramètres invalides lors de l'appel de -updateConfigurationFile" << endl;
		return false;
	}

	// Dossier de l'application
	string fullName(appName), path("");	
#ifdef _DEBUG
	fullName = "d:\\ldapTools\\ldap2File.exe";
#endif // _DEBUG

	fullName = sFileSystem::split(fullName, path);

	if (0 == path.length()) {
		cout << "Erreur - Pas de nom de dossier dans le nom de l'application" << endl;
		return false;
	}

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
	pugi::xml_node childNode = xmlConf.findChildNode(paramsRoot, XML_CONF_FOLDER_NODE, XML_CONF_FOLDER_OS_ATTR, xmlConf.expectedOS());
	
	// Trouvé ?
	if (!IS_EMPTY(childNode.name())) {
		string val = childNode.first_child().value();
#ifdef _DEBUG
		cout << "Dossier de l'application : " << val << endl;
#endif _DEBUG

		// Le dossier doit exister !
		if (!val.length() || !sFileSystem::exists(val)){

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
	path = sFileSystem::merge(path, STR_FOLDER_LOGS);
	pugi::xml_node logNode = paramsRoot.child(XML_CONF_LOGS_NODE);
	if (IS_EMPTY(logNode.name())) {
		cout << "Erreur - pas de noeud " << XML_CONF_LOGS_NODE << " dans le fichier '" << confFile << "'" << endl;
		return false;
	}

	childNode = xmlConf.findChildNode(logNode, XML_CONF_LOGS_FOLDER_NODE, XML_CONF_FOLDER_OS_ATTR, xmlConf.expectedOS());

	// Trouvé ?
	//
	if (!IS_EMPTY(childNode.name())) {
		string val = childNode.first_child().value();
		
#ifdef _DEBUG
		cout << "Dossier des logs : " << val << endl;
#endif // _DEBUG

		if (!sFileSystem::exists(val)) {
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
		cout << "\t- Pas de dossier précisé => rien à faire" << endl;
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
	cout << "\n\t" << APP_FULL_NAME << " {files or folder} [-base:{folder}] [-d:{folder}] [-f:{freq}] [-o:{output filename}] [-c] [-s]" << endl;
	cout << "\n\t\t. {files or folder} : Listes des fichiers de commande à traiter. Lorsqu'un seul nom est fourni, et qu'il s'agit d'un dossier, tout le contenu du dossier sera traité (identique à -d:{folder})" << endl;
	cout << "\n\t\t. -base:{folder} : Le dossier 'folder' est considéré comme le dossier de l'application. Par défaut, le dossier de l'application est celui dans lequel se trouve le binaire." << endl;
	cout << "\n\t\t. -d:{folder} : Analyse de tous les fichiers de commandes contenus dans le dossier {folder}" << endl;
	cout << "\n\t\t. -f:{freq} : Analyse périodique des fichiers et des dossiers. Freq est la fréquence d'analyse en minutes" << endl;
	cout << "\n\t\t. -o:{output-file} : Le fichier généré sera renommé en {output-file] même si le fichier de commande indique une autre destination." << endl;
	cout << "\n\t\t . -c : Suppression du fichier de commande après traitement." << endl;
	cout << "\n\t\t.  -s : Windows uniquement = pas de MessageBox" << endl;
	
	cout << "\n\tExemples d'appels:" << endl;
	
	cout << "\n\tAnalyse et traitement des fichiers de commande du dossier c:\\my datas\\test toutes les 30 minutes:" << endl;
	cout << "\n\t\tldap2File.exe -d:\"c:\\my datas\\test\" -f:30" << endl;

	cout << "\n\tTraitement du fichier '~/ldap2Files/datas/dsun.xml'; le dossier de l'application étant '/shared/cmdFiles" << endl;
	cout << "\n\t\tldap2File -base /shared/cmdFiles ~/ldap2Files/datas/dsun.xml" << endl;

	cout << "\n\tTraitement des 5 fichiers passes en ligne de commande ! " << endl;
	cout << "\n\t\tldap2File.exe  file1.xml file2.xml file3.xml file4.xml file5.xml" << endl;
}

// Point d'entrée du programme
//
int main(int argc, const char* argv[]) {

#ifdef _WIN32
	// On bascule la console en UTF8 !!!
	SetConsoleOutputCP(CP_UTF8);
	setvbuf(stdout, nullptr, _IOFBF, 1000);
#endif // _WIN32

	// Nom complet de l'application
	//
	string fullAppName(argv[0]);
	
	// Pas de chemin
	size_t pathPos = fullAppName.rfind(FILENAME_SEP);
	if (fullAppName.npos == pathPos) {
		string path = sFileSystem::current_path();
		fullAppName = sFileSystem::merge(path, fullAppName);
	}

#ifdef _WIN32
	// L'extension ?
#endif _WIN32
	
	// Binaire et sa version
	//
	string binName(fullAppName);
	char sep(FILENAME_SEP);
	size_t pos(0);
	if (binName.npos != (pos = binName.rfind(sep))) {
		binName = binName.substr(pos + 1);
	}

	if (binName.npos != (pos = binName.rfind("."))) {
		binName = binName.substr(0, pos);
	}

	cout << binName << " - Version " << APP_RELEASE;
#ifdef _DEBUG
	cout << " - DEBUG";
#endif |DEBUG
	cout << endl;

	cout << "Copyright © " << APP_COPYRIGHT << endl;

	// Paramètres de la ligne de commandes
	//
	if (argc < 2) {
		_usage();
		return 1;
	}

	string error("");
	if (false == _checkCurrentVersion(error)) {
		if (error.length()) {
			cout << error;
			return 1;
		}

		// Mise à jour du fichier de configuration
		if (false == _updateConfigurationFile(fullAppName.c_str())) {
			return 1;
		}
	}

	// Lancement de l'application
	//
#ifdef _WIN32
	bool verbose(true);
#endif // _WIN32

	// Vérification de la ligne de commandes
	//
	int retCode(0);
	folders myFolders;		// Liste des dossiers utilisés par l'application	
	logFile logs;
	confFile configurationFile(&myFolders, &logs);
	string file("");

	try {
		// Dossier "de base" de l'application
		//
		string folder;

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
			//folder = argv[0];
			folder = fullAppName;
			size_t pos(folder.rfind(FILENAME_SEP));
			folder.resize(pos);
//#endif // _DEBUG
		}

		// Ajout du dossier de l'application ...
		myFolders.add(folders::FOLDER_TYPE::FOLDER_APP, folder);
		myFolders.add(folders::FOLDER_TYPE::FOLDER_LOGS, STR_FOLDER_LOGS);				// sous dossier des logs
		myFolders.add(folders::FOLDER_TYPE::FOLDER_TEMPLATES, STR_FOLDER_TEMPLATES);	// sous dossier des modèles
		myFolders.add(folders::FOLDER_TYPE::FOLDER_TEMP, STR_FOLDER_TEMP);				// fichiers temporaires
		myFolders.add(folders::FOLDER_TYPE::FOLDER_OUTPUTS, STR_FOLDER_OUTPUTS);		// pour les fichiers générés

		file = sFileSystem::merge(folder, XML_CONF_FILE);

		// Ouverture du fichier de configuration
		//
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

		// Le dossier des logs doit exister
		folders::folder* logFolder = myFolders.find(folders::FOLDER_TYPE::FOLDER_LOGS);
		if (NULL == logFolder) {
			throw LDAPException("Le dossier des logs n'a pu être ouvert ou crée");
		}

		//cout << "Binaire : " << argv[0] << endl;
		cout << "Binaire : " << fullAppName << endl;

		cout << "Dossiers de l'application : " << endl;
		cout << "\t - app : " << myFolders.find(folders::FOLDER_TYPE::FOLDER_APP)->path() << endl;
		cout << "\t - logs : " << myFolders.find(folders::FOLDER_TYPE::FOLDER_LOGS)->path() << endl;
		cout << "\t - templates : " << myFolders.find(folders::FOLDER_TYPE::FOLDER_TEMPLATES)->path() << endl;
		cout << "\t - temporaires : " << myFolders.find(folders::FOLDER_TYPE::FOLDER_TEMP)->path() << endl;

		// Initialisation du fichier de logs
		//
#ifdef _WIN32
		logs.init(logFolder->path(), (lInfos.mode_ == LOGS_MODE_DEBUG) ? TRACE_LOGMODE_LIB__DEBUG : TRACE_LOGMODE_LIB__LOG);
		if (lInfos.fileName_.size()) {
			logs.setFileName(lInfos.fileName_.c_str());
		}
#endif // _WIN32

		logs.setFileAge(lInfos.duration_);	// JHB -> retrouver le corps de la méthode !!!

		logs.add(logFile::LOG, "=========================================================================");
#ifdef _DEBUG
		logs.add(logFile::LOG, "==== %s - version %s - %s", APP_SHORT_NAME, APP_RELEASE, APP_DESC);
#else
		logs.add(logFile::LOG, "==== %s - version %s - DEBUG - %s", APP_SHORT_NAME, APP_RELEASE, APP_DESC);
#endif // _DEBUG
		logs.add(logFile::LOG, "==== Copyright %s", APP_COPYRIGHT);
		logs.add(logFile::LOG, "Lancement de l'application");
		logs.add(logFile::DBG, "Logs en mode DEBUG");

		if (LOGS_DAYS_INFINITE != lInfos.duration_) {
			logs.add(logFile::DBG, "Conservation des logs %d jours", lInfos.duration_);
		}

		//logs.add(logFile::LOG, "Binaire : %s", argv[0]);
		logs.add(logFile::LOG, "Binaire : %s", fullAppName.c_str());
		logs.add(logFile::LOG, "Fichier de configuration : %s", file.c_str());
		logs.add(logFile::LOG, "Dossiers de l'application : ");
		logs.add(logFile::LOG, "\t- app : %s", myFolders.find(folders::FOLDER_TYPE::FOLDER_APP)->path());
		logs.add(logFile::LOG, "\t- templates : %s", myFolders.find(folders::FOLDER_TYPE::FOLDER_TEMPLATES)->path());
		logs.add(logFile::LOG, "\t- temporaires : %s", myFolders.find(folders::FOLDER_TYPE::FOLDER_TEMP)->path());
	}
	catch (LDAPException& e) {
		logs.add(logFile::ERR, "Erreur : %s", e.what());
		retCode = 1;
	}
	catch (...) {
		// Erreur inconnue
		logs.add(logFile::ERR, "Erreur inconnue");
		retCode = 1;
	}

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
								files.push_back(argv[index]);
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
			_getFolderContent(remoteFolder, files);
		}
		else {
			// Si un seul fichier, c'est peut être un dossier ...
			if (1 == files.size() && sFileSystem::is_directory(*files.begin())) {
					remoteFolder = (*files.begin());
					_getFolderContent(remoteFolder, files);
			}
		}

		bool done(0 == files.size());

		// Analyse des fichiers de commandes
		//
		DWORD currentLaunchTime(0);
		ldapBrowser requester(&logs, &configurationFile);
		RET_TYPE retType(RET_TYPE::RET_OK);
		while (!done) {
			currentLaunchTime = _getTickCount();

			for (list<string>::iterator it = files.begin(); it != files.end(); it++) {
				// Génération du fichier en question
				_now();
				cout << "[" << binName << "] " << (*it);
				if (configurationFile.openCommandFile((*it).c_str()) &&
					RET_TYPE::RET_OK == (retType = requester.browse())) {
					cout << " - [ok]";
					filesGenerated++;
				}
				else {
					switch (retType) {
					case RET_TYPE::RET_INVALID_PARAMETERS:
						cout << " - [erreur - paramètres invalides]";
						break;

					case RET_TYPE::RET_NON_BLOCKING_ERROR:
						cout << " - [erreur(s) non bloquante(s)]";
						filesGenerated++;	// L'erreur n'a pas empêchée la génération du fichier
						break;

					case RET_TYPE::RET_LDAP_ERROR:
						cout << " - [erreur LDAP]";
						break;

					case RET_TYPE::RET_UNABLE_TO_SAVE:
						cout << " - [erreur de fichier]";
						break;

					case RET_TYPE::RET_BLOCKING_ERROR:
					default:
						cout << " - [erreur(s)]";
						break;
					}
				}

				cout << endl;

				// Suppression du fichier de commandes
				if (removeFile) {
					sFileSystem::remove((*(files.begin())).c_str());

					// Il ne peut pas y avoir d'analyse régulière ...
					freq = 0;
				}
			}

			if (freq) {
				// On attend un peu
				_now();
				cout << "Sleep " << (freq / 60000) << " min" << endl;
				logs.add(logFile::LOG, "Sleep %d min.", freq / 60000);

				int duration = _getTickCount() - currentLaunchTime;	// Durée des traitements précédents
				duration = (duration > freq ? freq : freq - duration);
#ifdef _WIN32
				Sleep(duration);
#else
				sleep(duration);
#endif //#ifdef _WIN32

				// Analyse du dossier ?
				if (remoteFolder.size()) {
					_getFolderContent(remoteFolder, files);
					done = (0 == files.size());
				}
			}
			else {
				done = true;
			}
		}
	}
	catch (LDAPException& e) {
		logs.add(logFile::ERR, "Erreur : %s", e.what());
		retCode = 1;
	}
	catch (...) {
		// Erreur inconnue
		logs.add(logFile::ERR, "Erreur inconnue");
		retCode = 1;
	}


#ifdef _WIN32
	if (verbose) {
		char message[200];
		sprintf_s(message, 199, "%d fichier(s) genere(s)", filesGenerated);

		MessageBox(NULL, message, APP_SHORT_NAME, MB_ICONINFORMATION);
	}
#else
	cout << filesGenerated << " fichier" << (filesGenerated > 1 ? "s " : " ") << "crée" << (filesGenerated > 1 ? "s" : " ");
#endif // _WIN32

	// Fin
	logs.add(logFile::LOG, "%d / %d fichier(s) crée(s)", filesGenerated, files.size());
	logs.add(logFile::LOG, "Fermeture de l'application");
	logs.add(logFile::LOG, "=========================================================================");

	return retCode;
}

// EOF