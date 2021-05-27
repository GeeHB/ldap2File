//---------------------------------------------------------------------------
//--
//--	FICHIER	: charUtils.h
//--
//--	AUTEUR	: Jérôme Henry-Barnaudière - JHB
//--
//--	PROJET	:
//--
//--	DATE	: 19/05/2021
//--
//---------------------------------------------------------------------------
//--
//--	DESCRIPTION:
//--
//--		Gestion des chaines de caractères (fonctions, encodage)
//--
//---------------------------------------------------------------------------
//--
//--	MODIFICATIONS:
//--	-------------
//--
//--	08/12/2014 - JHB - Création
//--
//--	15/01/2015 - JHB - Encodage et/ou décodage
//--
//--	17/01/2015 - JHB - Retrait des accents
//--
//--	15/02/2016 - JHB - Version 1.3
//--
//--	27/01/2017 - JHB - Version 2.3 - Ajout des méthodes spécifiques à VS
//--					+ Gestion plus ouverte de l'UTF8
//--					+ Encodage base64
//--					+ Ajouts de caractères
//--					+ Gestion des sauts de ligne
//--
//--	27/06/2017 - JHB - Version 2.5.2
//--					+ Fonctions d'encodage/décodage
//--
//--	15/02/2016 - JHB - Version 1.3
//--
//--	12/03/2018 - JHB - Version 18.3.1 - Bugs d'encodage
//--
//--	13/03/2020 - JHB - Version 20.3.2 - Utilisation des API de conversion Windows
//--
//--	05/04/2020 - JHB - Version 20.4.6 - Corrections
//--
//--	19/05/2021 - JHB - Version 21.5.7 - Corrections & modification de prototypes
//--
//---------------------------------------------------------------------------

//
// TODO
//			- Ajouter la génération et le découpage des lignes pour LDIF
//

#ifndef __JHB_CHAR_UTILS_h__
#define __JHB_CHAR_UTILS_h__    1

// #define __JHB_USE_CPP__

#include <sstream>
#include <iomanip>
#include <deque>
#include <string>

#include <commonTypes.h>

//---------------------------------------------------------------------------
//--
//--		Constantes publiques
//--
//---------------------------------------------------------------------------

// Une ligne MIME::base64 ne doit pas dépasser 78 caractères hors CRLF (cf. RFC5322)
//  l'encodage base64 transforme 3 octets source en 4 octets dest
//
#define RFC5322_MAX_LINE_LEN			78

//  => Taille max. d'une ligne avant encodage = 57 car ( 57 *4/3 = 76)
//
#define RFC5322_BASE64_MAX_LINE_LEN		57


//	Les caractères pour l'encodage base64
//
static const std::string base64_chars =
					"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
					"abcdefghijklmnopqrstuvwxyz"
					"0123456789+/";

#define RFC5322_ATOM	"!#$%&'*+-/=?^_`{|}~"

#ifndef WIN32
typedef unsigned char __int8;
typedef short __int16;
//typedef int int;
#endif // _WIN32

// Sauts de ligne
//
#define CHAR_LF			0x0A
#define CHAR_CR			0x0D


//---------------------------------------------------------------------------
//--
//--		Classe charUtils
//--
//---------------------------------------------------------------------------

class charUtils
{
	// Méthodes publiques
	//
public:

	// Format de la source (externe)
	//
	enum class SOURCE_FORMAT {
		UTF8 = 0,				// Rien à faire ...
		ISO_8859_1 = 1,			// Encodage Latin
		ISO_8859_15 = 2,		// Encodage Latin compatible avec l'alphabet Français (identique à 8859-1 sauf 8 car.)
		UNKNOWN = 0x0FFFFFFF	// ???
	};

	// Constructions
	charUtils(SOURCE_FORMAT source)
		: initialized_{ false }, eol_{ "\n" }{
		sourceFormat(source);
	}
	charUtils()
		: initialized_{ false }, format_{ SOURCE_FORMAT::UNKNOWN }, eol_{ "\n" }
	{}

	// Destruction
	virtual ~charUtils()
	{ _clear(false); }

	// Définition de l'encodage source
	//
	void sourceFormat(SOURCE_FORMAT source, bool quotedPrintable = false/*, bool extendedLatin = true*/);

	// Saut de ligne à utiliser
	enum class FORMAT_EOL { EOL_CR, EOL_LF, EOL_CRLF };

	std::string eol(FORMAT_EOL eType);
	std::string eol()
	{ return eol_; }

	//
	// Fonctions d'encodage / décodage
	//

	// UTF8
	//

	// Version qui modifient l'entrée
	//

	// UTF8 => Ascii
	bool convert_fromUTF8(std::string& source);

	// Ascii => UTF8
	bool convert_toUTF8(std::string& source, bool MIMEEncode);

	// Versions neutres (la chaine en entrée n'est pas modifiée)
	//
	std::string toUTF8(const char* value, size_t len);
	std::string toUTF8(const char* value){
		if (value){
			std::string inter(value);
			return toUTF8(inter);
		}

		return "";
	}
	std::string toUTF8(const std::string& value){
		std::string str = toUTF8(value.c_str(), value.size());
		return str;
	}

	// Codage UTF8 valide ?
	static bool isValidUTF8(const char* source);
	static bool isValidUTF8(const std::string source)
	{ return charUtils::isValidUTF8(source.c_str()); }
	//bool utf8_check_is_valid(const std::string& string);

	static bool isPureASCII(const char* str);
	static bool isPureASCII(const std::string str)
	{ return charUtils::isPureASCII(str.c_str()); }

	// Accès
    static size_t utf8_realIndex(const std::string& source, size_t index);

	// Base 64
	//

	// Encodage et décodage base64 - corps du message et PJ
	std::string toBase64(unsigned char const*, size_t len);
	std::string toBase64(std::string& source)
	{ return toBase64((unsigned char const*)source.c_str(), source.length()); }
	std::string fromBase64(std::string const& s);

	// Utilitaires
	std::string text2Base64(const std::string& source);
	std::string file2Base64(const std::string& fileName);

	// UTF8 puis "Quoted" ou "Base64"
	enum class MIME_ENCODE { QUOTED = 0, BASE64};

	std::string toMIMEUTF8(const std::string& source, const MIME_ENCODE mEncode);

	// Helpers ...
	//
    std::string createKeyValueLine(const char* token, const std::string& value, size_t maxLineLength){
		return createKeyValueLine(token, value, maxLineLength, eol_);
	}
	std::string createKeyValueLine(const char* token, const std::string& value, size_t maxLineLength, const std::string& cEol);
	std::string splitLine(const std::string& source, size_t maxLineLength, const std::string& cEol);
	std::string splitLine(const std::string& source, size_t maxLineLength){
		return splitLine(source, maxLineLength, eol_);
	}
	std::string splitLine(const char* token, const std::string& source, size_t maxLineLength){
		return splitLine(token, source, maxLineLength, eol_);
	}
	std::string splitLine(const char* token, const std::string& source, size_t maxLineLength, const std::string& cEol){
		std::string fLine("");
		if (token && strlen(token)){
			fLine = token;
			fLine += ": ";
		}
		fLine += source;
		return splitLine(fLine, maxLineLength, cEol);
	}

	//
	// Bibliothèque de fonctions/méthodes
	//

	// Gestion des caractères accentués
	static std::string removeAccents(const char* source);
	static std::string removeAccents(std::string & source)
	{ return removeAccents(source.c_str()); }
	static bool equalWithoutAccents(std::string& left, std::string& right)
	{ return equalWithoutAccents(left, right.c_str()); }
	static bool equalWithoutAccents(std::string& left, const char *right);
	static std::string upString(std::string& source, bool removeAccents = false);

	// Nombre d'occurence d'une chaine dans une autre
	static size_t countOf(std::string& str, char searched, size_t from);

	// Fonctions de manipulation des chaines de caractères
	static std::string strupr(const char* source);
	static char* strupr(char* source);
	static std::string strlwr(const char* source);
	static char* strlwr(char* source);
	static int stricmp(const char* left, const char* right);

	static std::string clean(const std::string& source, const char* toremove);

	// Conversion
	static std::string itoa(int num, int radix = 10, int digits = 0);
	static char* itoa(int num, char* buffer, int radix, int digits = 0);

	// Taille
	static std::string shorten(const std::string& source, const size_t max);

	// Nettoyage
	//

	// Retrait des guillemets
	static std::string cleanName(const char* input) {
		if (IS_EMPTY(input)) {
			std::string inter("");
			return inter;
		}
		std::string inter(input);
		return charUtils::cleanName(inter);
	}
	static std::string cleanName(const std::string& src) {
		std::string inter(src);
		if ('\"' == inter[0]) {
			size_t len = inter.length();
			return ('\"' == inter[len - 1]) ? inter.substr(1, len - 2) : inter.substr(1, len - 1);
		}
		return inter;
	}


// Méthodes privées
protected:

	// Libérations
	void _clear(bool reuse)
	{}

	static inline bool _is_base64(unsigned char c){
		return (isalnum(c) || (c == '+') || (c == '/'));
	}

	// Conversions
	std::string _UTF8toISO8859_1(const std::string& str);
	std::string _ISO8859_1toUTF8(std::string &str);

#ifdef _WIN32
	std::string _win32_UTF8toISO8859_1(const std::string& str) {
		return _win32_UTF8Convert(str, CP_UTF8, CP_ACP);
	}
	std::string _win32_ISO8859_1toUTF8(std::string &str) {
		return _win32_UTF8Convert(str, CP_ACP, CP_UTF8);
	}

	std::string _win32_UTF8Convert(const std::string& str, UINT from, UINT to);
#endif // _WIN32

// Données membres
//
protected :
	bool					initialized_;
	SOURCE_FORMAT			format_;
	std::string				eol_;		// Saut de ligne
};

#endif // __JHB_CHAR_UTILS_h__

// EOF
