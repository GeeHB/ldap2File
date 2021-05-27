//---------------------------------------------------------------------------
//--
//--	FICHIER	: CURLTools.h
//--
//--	AUTEUR	: Jérôme Henry-Barnaudière - JHB
//--
//--	PROJET	: xxx
//--
//--    COMPATIBILITE : Win32 | Linux (Fedora 33)
//--
//---------------------------------------------------------------------------
//--
//--	DESCRIPTION:
//--
//--		Définitions communes pour les objets utilisant la librairies CURL
//--
//--    REMARQUES:
//--
//--        Linux / Linker : -lcurl
//--
//---------------------------------------------------------------------------
//--
//--	MODIFICATIONS:
//--	-------------
//--
//--	06/07/2020 - JHB - Création
//--
//--	11/05/2021 - JHB - Corrections / Compatibilité linux
//--
//---------------------------------------------------------------------------


#ifndef __JHB_CURL_TOOLS_H__
#define __JHB_CURL_TOOLS_H__

#include <exception>
#ifdef _WIN32
#include "./curl/curl.h"
#else
#include <curl/curl.h>
#endif // _WIN32
#include <algorithm>
#include <atomic>
#include <cstddef>  // std::size_t
#include <cstdio>   // snprintf
#include <cstdlib>
#include <cstring>  // strerror, strlen, memcpy, strcpy
#include <ctime>
#ifndef LINUX
#include <direct.h>  // mkdir
#endif
#include <stdarg.h>  // va_start, etc.
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>  // std::unique_ptr
#include <mutex>
#include <sstream>
#include <string>
#include <vector>

//---------------------------------------------------------------------------
//--
//--    Constantes puibliques
//--
//---------------------------------------------------------------------------

// Protocoles
//

// smtp
#ifndef IPPROTO_SMTP
#define IPPROTO_SMTP	"smtp"
#define IPPROTO_SMTPS	"smtps"
#endif // IPPROTO_SMTP

// ftp
#ifndef IPPROTO_FTP
#define IPPROTO_FTP     "ftp"
#define IPPROTO_FTPS    "ftps"
#define IPPROTO_FTPES   "ftpes"
#define IPPROTO_SFTP    "sftp"
#endif // IPPROTO_FTP

#ifndef IPPROTO_URL_SEP
#define IPPROTO_URL_SEP "://"
#endif // IPPROTO_URL_SEP

//
// Messages d'erreur
//

// Log messages

#define LOG_ERROR_INVALID_PARAMETERS        "Paramètres invalides"
#define LOG_ERROR_EMPTY_HOST_MSG            "hostnmae non renseigné"

#define LOG_ERROR_CURL_ALREADY_INIT_MSG     "La session CURL est déjà initialisée"
#define LOG_ERROR_CURL_NOT_INIT_MSG         "CURL non-intialialisé"
#define LOG_ERROR_CURL_NOT_INITIALIZED      "Erreur lors de l'initiialisation de CURL"

#define LOG_ERROR_CURL_REMOVE_FORMAT        "Impossible de supprimer le fichier '%s' (Erreurr = %d | %s)"
#define LOG_ERROR_CURL_VERIFYURL_FORMAT     "Impossible de se connecter au dossier '%s' (Erreurr = %d |  %s)"
#define LOG_ERROR_CURL_FILETIME_FORMAT      "Impossible de récupérer les informations pour le fichier '%s' (Erreur = %d | %s)"
#define LOG_ERROR_CURL_GETFILE_FORMAT       "Impossible de télécharger le fichier '%s/%s' (Erreur = %d | %s)"
#define LOG_ERROR_CURL_UPLOAD_FORMAT        "Impossible de téléverser le fichier '%s' (Erreur = %d | %s)"
#define LOG_ERROR_CURL_FILELIST_FORMAT      "Impossible de se connecter du dossier distant '%s' (Erreur = %d | %s)"
#define LOG_ERROR_CURL_GETWILD_FORMAT       "Impossible de télécharger %s/%s (Erreur = %d | %s)"
#define LOG_ERROR_CURL_GETWILD_REC_FORMAT   "Erreur lors de l'import '%s' vers '%s'"
#define LOG_ERROR_CURL_MKDIR_FORMAT         "Impossible de créer le dossier distant '%s' (Erreurr = %d | %s)"
#define LOG_ERROR_CURL_RMDIR_FORMAT         "Impossible de supprimer le dossier distant '%s' (Erreur = %d | %s)"

//
// Définitions dans l'espace de noms
//

namespace jhbCURLTools {

    class CURLException : public std::exception
    {
    public:
        CURLException(const char* message)
        {
            message_ = message;
        }
        CURLException(std::string& message)
        {
            message_ = message;
        }

        // Message d'erreur
        virtual const char* what() const throw()
        {
            return message_.c_str();
        }

    protected:
        std::string message_;       // Le message
    };

    class CURLHandle {

    public:
        CURLHandle(CURLHandle const&) = delete;
        CURLHandle(CURLHandle&&) = delete;

        ~CURLHandle()
        {
            curl_global_cleanup();
        }

        CURLHandle& operator=(CURLHandle const&) = delete;
        CURLHandle& operator=(CURLHandle&&) = delete;

        static CURLHandle& instance() {
            static CURLHandle inst{};
            return inst;
        }

    private:
        CURLHandle() {
            const auto eCode = curl_global_init(CURL_GLOBAL_ALL);
            if (eCode != CURLE_OK) {
                throw std::runtime_error{ "Erreur lors de l'initialisation de libCURL" };
            }
        }
    };

};  // namespace jhbCURLTools

#endif // __JHB_CURL_TOOLS_H__

// EOF
