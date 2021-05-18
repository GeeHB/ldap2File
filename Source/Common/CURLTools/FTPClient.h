//---------------------------------------------------------------------------
//--
//--	FICHIER	: FTPClient.h
//--
//--	AUTEUR	: Jérôme Henry-Barnaudière
//--
//--	DATE	: 11/05/2021
//--
//--    COMPATIBILITE : Win32 | Linux (Fedora 34 et supérieures)
//--
//---------------------------------------------------------------------------
//--
//--	DESCRIPTION:
//--
//--		Définition de la classes FTPClient
//--		pour les transfert en FTP
//--
//--    REMARQUES:
//--
//--        Linux : pour linker ajouter -lcurl (install curl-dev ou curl-devel)
//--
//---------------------------------------------------------------------------
//--
//--	MODIFICATIONS:
//--	-------------
//--
//--	xx/xx/xxxx - V0 Mohamed Amine Mzoughi <mohamed-amine.mzoughi@laposte.net>
//--
//--	06/07/2020 - JHB - devient jhbCURLTools::FTPClient
//--
//--    11/05/2021 - JHB - Corrections & compatibilité Linux
//--
//---------------------------------------------------------------------------

#ifndef __JHB_CUPS_FTP_CLIENT_h__
#define __JHB_CUPS_FTP_CLIENT_h__

#include "CURLTools.h"

#define FTPCLIENT_VERSION "FTPCLIENT_VERSION_1.0.2"

// Message d'erreur
//
#define LOG_ERROR_FILE_UPLOAD_FORMAT    "[FTPClient::UploadFile()] - Impossible d'ouvrir '%s'"
#define LOG_ERROR_FILE_GETFILE_FORMAT   "[FTPClient::DownloadFile()] - Impossible d'ouvrir '%s'"
#define LOG_ERROR_DIR_GETWILD_FORMAT    "[FTPClient::DownloadWildcard()] - '%s' n'est pas un dossier ou n'existe pas"

namespace jhbCURLTools {

class FTPClient {
  public:
   // Public definitions
   using ProgressFnCallback = std::function<int(void *, double, double, double, double)>;
   //using LogFnCallback      = std::function<void(const std::string &)>;

   // Used to download many items at once
   struct WildcardTransfersCallbackData {
      std::ofstream ofsOutput;
      std::string strOutputPath;
      std::vector<std::string> vecDirList;
      // will be used to call GetWildcard recursively to download subdirectories
      // content...
   };

   // Progress Function Data Object - parameter void* of ProgressFnCallback
   // references it
   struct ProgressFnStruct {
      ProgressFnStruct() : dLastRunTime(0), pCurl(nullptr), pOwner(nullptr) {}
      double dLastRunTime;
      CURL *pCurl;
      /* owner of the FTPClient object. can be used in the body of the progress
       * function to send signals to the owner (e.g. to update a GUI's progress
       * bar)
       */
      void *pOwner;
   };

   // See Info method.
   typedef struct FileInfo {
      time_t tFileMTime;
      double dFileSize;
   }FTPFILEINFO;

   enum class SETTINGS_FLAG {
      NO_FLAGS   = 0x00,
      ENABLE_LOG = 0x01,
      ENABLE_SSH = 0x02,  // only for SFTP
      ALL_FLAGS  = 0xFF
   };

   enum class FTP_PROTOCOL : unsigned char {
      // These three protocols below should not be confused with the SFTP
      // protocol. SFTP is an entirely different file transfer protocol
      // that runs over SSH2.
      FTP,  // Plain, unencrypted FTP that defaults over port 21. Most web browsers
            // support basic FTP.

      FTPS, /* Implicit SSL/TLS encrypted FTP that works just like HTTPS.
             * Security is enabled with SSL as soon as the connection starts.
             * The default FTPS port is 990. This protocol was the first version
             * of encrypted FTP available, and while considered deprecated, is
             * still widely used. None of the major web browsers support FTPS. */

      FTPES, /* Explicit FTP over SSL/TLS. This starts out as plain FTP over port
              * 21, but through special FTP commands is upgraded to TLS/SSL
              * encryption. This upgrade usually occurs before the user
              * credentials are sent over the connection. FTPES is a somewhat
              * newer form of encrypted FTP (although still over a decade old),
              * and is considered the preferred way to establish encrypted
              * connections because it can be more firewall friendly. None of the
              * major web browsers support FTPES. */

      SFTP
   };

   /* Please provide your logger thread-safe routine, otherwise, you can turn off
    * error log messages printing by not using the flag ALL_FLAGS or ENABLE_LOG
    */
   //explicit FTPClient(LogFnCallback oLogger = [](const std::string &) {});

   FTPClient();
   virtual ~FTPClient();

   // copy constructor and assignment operator are disabled
   FTPClient(const FTPClient &) = delete;
   FTPClient &operator=(const FTPClient &) = delete;

   // allow constructor and assignment operator are disabled
   FTPClient(FTPClient &&) = delete;
   FTPClient &operator=(FTPClient &&) = delete;

   // Setters - Getters (for unit tests)
   void SetProgressFnCallback(void *pOwner, const ProgressFnCallback &fnCallback);
   void SetProxy(const std::string &strProxy);
   inline void SetTimeout(const int &iTimeout)
   { iCurlTimeout_ = iTimeout; }
   inline void SetActive(const bool &bEnable)
   { bActive_ = bEnable; }
   inline void SetNoSignal(const bool &bNoSignal)
   { bNoSignal_ = bNoSignal; }
   inline auto GetProgressFnCallback() const
   { return fnProgressCallback_.target<int (*)(void *, double, double, double, double)>(); }
   inline void *GetProgressFnCallbackOwner() const
   { return ProgressStruct_.pOwner; }
   inline const std::string &GetProxy() const
   { return strProxy_; }
   inline const int GetTimeout() const
   { return iCurlTimeout_; }
   inline const unsigned GetPort() const
   { return uPort_; }
   inline const bool GetActive()
   { return bActive_; }
   inline const bool GetNoSignal() const
   { return bNoSignal_; }
   inline const std::string &GetURL() const
   { return strServer_; }
   inline const std::string &GetUsername() const
   { return strUserName_; }
   inline const std::string &GetPassword() const
   { return strPassword_; }
   /*
   inline const unsigned char GetSettingsFlags() const
   { return eSettingsFlags_; }
   */
   inline const SETTINGS_FLAG GetSettingsFlags() const
   { return eSettingsFlags_; }
   inline const FTP_PROTOCOL GetProtocol() const
   { return eFtpProtocol_; }

   // Session
   const void InitSession(const std::string &strHost, const unsigned &uPort, const std::string &strLogin, const std::string &strPassword,
                          const FTP_PROTOCOL &eFtpProtocol = FTP_PROTOCOL::FTP, const SETTINGS_FLAG &SettingsFlags = SETTINGS_FLAG::ALL_FLAGS);
   virtual const void CleanupSession();
   const CURL *GetCurlPointer() const
   { return pCurlSession_; }

   // FTP requests
   const void CreateDir(const std::string &strNewDir) const;

   const bool RemoveDir(const std::string &strDir) const;

   const void RemoveFile(const std::string &strRemoteFile) const;

   /* Checks a single file's size and mtime from an FTP server */
   const bool Info(const std::string &strRemoteFile, FTPFILEINFO &oFileInfo) const;

   const void List(const std::string &strRemoteFolder, std::string &strList, bool bOnlyNames = true) const;

   const void DownloadFile(const std::string &strLocalFile, const std::string &strRemoteFile) const;

   const void DownloadFile(const std::string &strRemoteFile, std::vector<char> &data) const;

   const void DownloadWildcard(const std::string &strLocalDir, const std::string &strRemoteWildcard) const;

   const void UploadFile(const std::string &strLocalFile, const std::string &strRemoteFile, const bool &bCreateDir = false) const;

   // SSL certs
   void SetSSLCertFile(const std::string &strPath) { strSSLCertFile_ = strPath; }
   const std::string &GetSSLCertFile() const { return strSSLCertFile_; }

   void SetSSLKeyFile(const std::string &strPath) { strSSLKeyFile_ = strPath; }
   const std::string &GetSSLKeyFile() const { return strSSLKeyFile_; }

   void SetSSLKeyPassword(const std::string &strPwd) { strSSLKeyPwd_ = strPwd; }
   const std::string &GetSSLKeyPwd() const { return strSSLKeyPwd_; }

#ifdef DEBUG_CURL
   static void SetCurlTraceLogDirectory(const std::string &strPath);
#endif

  private:
   /* common operations are performed here */
   inline const CURLcode Perform() const;
   inline std::string ParseURL(const std::string &strURL) const;

   // Curl callbacks
   static size_t WriteInStringCallback(void *ptr, size_t size, size_t nmemb, void *data);
   static size_t WriteToFileCallback(void *ptr, size_t size, size_t nmemb, void *data);
   static size_t ReadFromFileCallback(void *ptr, size_t size, size_t nmemb, void *stream);
   static size_t ThrowAwayCallback(void *ptr, size_t size, size_t nmemb, void *data);
   static size_t WriteToMemory(void *ptr, size_t size, size_t nmemb, void *data);

   // Wildcard transfers callbacks
   static long FileIsComingCallback(struct curl_fileinfo *finfo, WildcardTransfersCallbackData *data, int remains);
   static long FileIsDownloadedCallback(WildcardTransfersCallbackData *data);
   static size_t WriteItCallback(char *buff, size_t size, size_t nmemb, void *cb_data);

   // String Helpers
   static std::string StringFormat(std::string strFormat, ...);
   static void ReplaceString(std::string &strSubject, const std::string &strSearch, const std::string &strReplace);

// Curl Debug informations
#ifdef DEBUG_CURL
   static int DebugCallback(CURL *curl, curl_infotype curl_info_type, char *strace, size_t nSize, void *pFile);
   inline void StartCurlDebug() const;
   inline void EndCurlDebug() const;
#endif

   std::string          strUserName_;
   std::string          strPassword_;
   std::string          strServer_;
   std::string          strProxy_;

   bool                 bActive_;  // For active FTP connections
   bool                 bNoSignal_;
   unsigned             uPort_;

   FTP_PROTOCOL         eFtpProtocol_;
   SETTINGS_FLAG        eSettingsFlags_;

   // SSL
   std::string          strSSLCertFile_;
   std::string          strSSLKeyFile_;
   std::string          strSSLKeyPwd_;

   mutable CURL         *pCurlSession_;
   int                  iCurlTimeout_;

   // Progress function
   ProgressFnCallback   fnProgressCallback_;
   ProgressFnStruct     ProgressStruct_;
   bool                 bProgressCallbackSet_;

#ifdef DEBUG_CURL
   static std::string s_strCurlTraceLogDirectory;
   mutable std::ofstream m_ofFileCurlTrace;
#endif
   CURLHandle &hCurl_;
};

};  // namespace jhbCURLTools

#endif // __JHB_CUPS_FTP_CLIENT_h__

// EOF
