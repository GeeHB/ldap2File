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
//-- 20/07/2020 - JHB - Version 20.7.22
//--
//---------------------------------------------------------------------------

#include "sharedConsts.h"
#include "ldapBrowser.h"

#include <fileSystem.h>

#ifndef _WIN32
#include <dirent.h>		// Gestion des repertoires
#include <ctime>
#endif // #ifndef _WIN32

// Prototypes
//
void _getFolderContent(const string& source, list<string>& content);
void _now();
int _getTickCount();

// Point d'entrée du programme
//
int main(int argc, const char * argv[]){
#ifdef WIN32
	bool verbose(true);
#endif // WIN32

	// Vérification de la ligne de commandes
	//
	int retCode(0);

	if (argc < 2){
		cout << "Mauvaise ligne de commande";
		return 1;
	}

	// Binaire et sa version
	//
	string binName(argv[0]);
	char sep(FILENAME_SEP);
	size_t pos(0);
	if (binName.npos != (pos = binName.rfind(sep))){
		binName = binName.substr(pos + 1);
	}

	if (binName.npos != (pos = binName.rfind("."))){
		binName = binName.substr(0, pos);
	}

	cout << binName << " - Version " << APP_RELEASE << endl;
	cout << "Copyright " << APP_COPYRIGHT << endl;

	logFile logs;
	confFile configurationFile(&logs);

	try{
		// Dossier de l'application
		//
		string folder;

		// Passé en ligne de commande ?
		for (int index = 1; 0 == folder.size() && index < argc; index++){
			if (argv[index] == strstr(argv[index], CMD_LINE_BASE_FOLDER)){
				folder = argv[index] + strlen(CMD_LINE_BASE_FOLDER) + 1;
			}
		}

		if (0 == folder.size()){
			// Sinon dans le dossier courant
/*
#ifdef _DEBUG
			folder = FOLDER_APP_DEFAULT;
#else
*/
			folder = argv[0];
			size_t pos(folder.rfind(FILENAME_SEP));
			folder.resize(pos);
//#endif // _DEBUG
		}

		string file(folder);
		file += FILENAME_SEP;
		file += XML_CONF_FILE;

		// Logs par défaut ...
#ifdef WIN32
		logs.init("C:\\ldapTools\\logs", TRACE_LOGMODE_LIB__DEBUG);
#endif // WIN32

		// Ouverture du fichier de configuration
		//
		if (!configurationFile.open(file.c_str())){
			return 1;
		}

		LOGINFOS lInfos;
		configurationFile.logInfos(lInfos);

		// Initialisation du fichier de logs
		//
#ifdef WIN32
		logs.init(lInfos.folder_.c_str(), (lInfos.mode_ == LOGS_MODE_DEBUG) ? TRACE_LOGMODE_LIB__DEBUG : TRACE_LOGMODE_LIB__LOG);
		if (lInfos.fileName_.size()){
			logs.setFileName(lInfos.fileName_.c_str());
		}
#endif // WIN32

		logs.setFileAge(lInfos.duration_);	// JHB -> retrouver le corps de la méthode !!!

		logs.add(logFile::LOG, _T("========================================================================="));
		logs.add(logFile::LOG, _T("==== %s - version %s - %s"), APP_SHORT_NAME, APP_RELEASE, APP_DESC);
		logs.add(logFile::LOG, _T("==== Copyright %s"), APP_COPYRIGHT);
		logs.add(logFile::LOG, _T("Lancement de l'application"));
		logs.add(logFile::DBG, _T("Logs en mode DEBUG"));

		if (LOGS_DAYS_INFINITE != lInfos.duration_){
			logs.add(logFile::DBG, _T("Conservation des logs %d jours"), lInfos.duration_);
		}

		logs.add(logFile::LOG, _T("Fichier de configuration : '%s'"), file.c_str());
	}
	catch (LDAPException& e){
		logs.add(logFile::ERR, "Erreur : %s", e.what());
		retCode = 1;
	}
	catch (...){
		// Erreur inconnue
		logs.add(logFile::ERR, "Erreur inconnue");
		retCode = 1;
	}

	list<string> files;
	size_t filesGenerated(0);

	try{
		int freq(0);
		bool removeFile(false);
		string remoteFolder("");

		//
		// Les fichiers à analyser sont soit passés en ligne de commande soit contenus dans un dossier
		//

		// Récupération de tous les noms de fichier en ligne de commande
		//
		for (int index=1; index < argc; index++){
			
#ifdef WIN32
			if (argv[index] == strstr(argv[index], CMD_NO_VERBOSE)){
				// Pas d'affichages
				verbose = false;
			}
			else{
#endif // WIN32
				if (argv[index] == strstr(argv[index], CMD_REMOVE_FILE)){
					// Suppression du ou des fichier(s)
					removeFile = true;
				}
				else{
					if (argv[index] == strstr(argv[index], CMD_LINE_FREQ)){
						// Analyse régulière
						freq = atoi(argv[index]+strlen(CMD_LINE_FREQ)) * 60000;
					}
					else{
						if (argv[index] == strstr(argv[index], CMD_LINE_DIR)){
							// Analyse d'un dossier complet
							remoteFolder = argv[index] + strlen(CMD_LINE_DIR);
						}
						else{
							if (argv[index] == strstr(argv[index], CMD_LINE_BASE_FOLDER)){
								// Déja lu ...
							}
							else{
								// Un fichier ...
								files.push_back(argv[index]);
							}
						}
					}
				}
			}
#ifdef WIN32
		}
#endif // WIN32

		// Analyse d'un dossier
		if (remoteFolder.size()){
			_getFolderContent(remoteFolder, files);
		}
		else{
			// Si un seul fichier, c'est peut être un dossier ...
			if (1 == files.size()){
#ifdef _WIN32
				DWORD dwAttrib = GetFileAttributes((*files.begin()).c_str());
				if ((dwAttrib != INVALID_FILE_ATTRIBUTES &&
					(dwAttrib & FILE_ATTRIBUTE_DIRECTORY))){
					remoteFolder = (*files.begin());
					_getFolderContent(remoteFolder, files);
				}
#endif // _WIN32
			}
		}

		bool done(0==files.size());

		// Analyse des fichiers de commandes
		//
		DWORD currentLaunchTime(0);
		ldapBrowser requester(&logs, &configurationFile);
		RET_TYPE retType(RET_TYPE::RET_OK);
		while (!done){
			currentLaunchTime = _getTickCount();

			for (list<string>::iterator it = files.begin(); it!=files.end(); it++){
				// Génération du fichier en question
				_now();
				cout << "[" << binName << "] " << (*it);
				if (configurationFile.openCommandFile((*it).c_str()) &&
					RET_TYPE::RET_OK == (retType = requester.browse())){
					cout << " - [ok]";
					filesGenerated++;
				}
				else{
					switch (retType){
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
					default :
						cout << " - [erreur(s)]";
						break;
					}
				}

				cout << endl;

				// Suppression du fichier de commandes
				if (removeFile){
#ifdef _WIN32
					DeleteFile((*(files.begin())).c_str());
#else
					remove((*(files.begin())).c_str());
#endif
					// Il ne peut pas y avoir d'analyse régulière ...
					freq = 0;
				}
			}

			if (freq){
				// On attend un peu
				_now();
				cout << "Sleep " << (freq / 60000) << " min" << endl;
				logs.add(logFile::LOG, "Sleep %d min.", freq / 60000);

				int duration = _getTickCount() - currentLaunchTime;	// Durée des traitements précédents
				duration = (duration > freq ? freq : freq - duration);
#ifdef WIN32
				Sleep(duration);
#else
				sleep(duration);
#endif //#ifdef WIN32

				// Analyse du dossier ?
				if (remoteFolder.size()){
					_getFolderContent(remoteFolder, files);
					done = (0 == files.size());
				}
			}
			else{
				done = true;
			}
		}
	}
	catch (LDAPException& e){
		logs.add(logFile::ERR, "Erreur : %s", e.what());
		retCode = 1;
	}
	catch (...){
		// Erreur inconnue
		logs.add(logFile::ERR, "Erreur inconnue");
		retCode = 1;
	}


#ifdef _WIN32
	if (verbose){
		char message[200];
		sprintf_s(message, 199, "%d fichier(s) genere(s)", filesGenerated);

		MessageBox(NULL, message, APP_SHORT_NAME, MB_ICONINFORMATION);
	}
#else
	cout << filesGenerated << " fichier" << (filesGenerated>1?"s ":" ") << "crée" << (filesGenerated>1?"s":" ");
#endif // _WIN32

	// Fin
	logs.add(logFile::LOG, _T("%d / %d fichier(s) crée(s)"), filesGenerated, files.size());
	logs.add(logFile::LOG, _T("Fermeture de l'application"));
	logs.add(logFile::LOG, _T("========================================================================="));

	return retCode;
}

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

// EOF
