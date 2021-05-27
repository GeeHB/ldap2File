//---------------------------------------------------------------------------
//--
//--	FICHIER	: charUtils.cpp
//--
//--	AUTEUR	: Jérôme Henry-Barnaudière - JHB
//--
//--	PROJET	:
//--
//--	DATE	: 19/05/2021
//--
//--    COMPATIBILITE : Win32 | Linux (Fedora 34 et supérieures)
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

#include "charUtils.h"

//---------------------------------------------------------------------------
//--
//--    Quelques constantes
//--
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
//--
//--		Classe charUtils
//--
//---------------------------------------------------------------------------

// Construction
//
void charUtils::sourceFormat(SOURCE_FORMAT source, bool quotedPrintable)
{
	// Déja fait ?
	if (initialized_){
		_clear(true);
	}

	format_ = source;

	// Les données sont été correctement initialisées
	initialized_ = true;
}

// Codage du saut de ligne
//
std::string charUtils::eol(FORMAT_EOL eType)
{
	eol_ = "";
	switch (eType){
	case FORMAT_EOL::EOL_CR:
		eol_ = CHAR_CR;
		break;
	case FORMAT_EOL::EOL_LF:
		eol_ = CHAR_LF;
		break;
	case FORMAT_EOL::EOL_CRLF:
	default:
#ifdef _WIN32
		eol_ = CHAR_LF;
#else
		eol_ = CHAR_CR;
		eol_ += CHAR_LF;
#endif // _WIN32
		break;
	}
	return eol_;
}

//
// UTF8
//

// UTF8 => char*
//
bool charUtils::convert_fromUTF8(std::string& source)
{
#ifdef _WIN32
	std::string out = _win32_UTF8toISO8859_1(source);
#else
	std::string out = _UTF8toISO8859_1(source);
#endif // #ifdef _WIN32

	source = out;
	return true;
}

// vers UTF8
//

// Conversion en UTF8 + encodage MIME (pour les mails)
//
bool charUtils::convert_toUTF8(std::string& source, bool MIMEEncode)
{
	// Conversion
#ifdef _WIN32
	std::string out = _win32_ISO8859_1toUTF8(source);
#else
	std::string out = _ISO8859_1toUTF8(source);
#endif // #ifdef _WIN32

	// En hexa ?
	if (MIMEEncode){
		std::string Xout("");
		char value[4];
		for (std::string::iterator it = out.begin(); it != out.end(); it++){
			sprintf(value, "=%02X", (*it));
			Xout.append(value);
			source = Xout;
		}
	}
	else{
		source = out;
	}

	// Terminé
	return true;
}

// Encodage UTF8 du texte
//
std::string charUtils::toUTF8(const char* value, size_t len)
{
	std::string source("");
	if (len && value){
		source = value;
		source.resize(len);
		convert_toUTF8(source, false);
	}

	return source;
}

// La chaine est-elle codée en UTF8 ?
//
bool charUtils::isValidUTF8(const char* source)
{
	if (!source) {
		// Une chaine vide et tout ce que l'on veut ....
		return true;
	}

	const unsigned char* bytes = (const unsigned char*)source;
	unsigned int cp;
	int num;

	while (*bytes != 0x00) {
		if ((*bytes & 0x80) == 0x00) {
			// U+0000 to U+007F
			cp = (*bytes & 0x7F);
			num = 1;
		}
		else if ((*bytes & 0xE0) == 0xC0) {
			// U+0080 to U+07FF
			cp = (*bytes & 0x1F);
			num = 2;
		}
		else if ((*bytes & 0xF0) == 0xE0) {
			// U+0800 to U+FFFF
			cp = (*bytes & 0x0F);
			num = 3;
		}
		else if ((*bytes & 0xF8) == 0xF0) {
			// U+10000 to U+10FFFF
			cp = (*bytes & 0x07);
			num = 4;
		}
		else {
			return false;
		}

		bytes += 1;
		for (int i = 1; i < num; ++i) {
			if ((*bytes & 0xC0) != 0x80) {
				return false;
			}
			cp = (cp << 6) | (*bytes & 0x3F);
			bytes += 1;
		}

		if ((cp > 0x10FFFF) ||
			((cp >= 0xD800) && (cp <= 0xDFFF)) ||
			((cp <= 0x007F) && (num != 1)) ||
			((cp >= 0x0080) && (cp <= 0x07FF) && (num != 2)) ||
			((cp >= 0x0800) && (cp <= 0xFFFF) && (num != 3)) ||
			((cp >= 0x10000) && (cp <= 0x1FFFFF) && (num != 4))) {
			return false;
		}
	}

	return true;
}
/*
bool charUtils::utf8_check_is_valid(const std::string& string)
{
    int c,i,ix,n,j;
    for (i=0, ix=string.length(); i < ix; i++)
    {
        c = (unsigned char) string[i];
        //if (c==0x09 || c==0x0a || c==0x0d || (0x20 <= c && c <= 0x7e) ) n = 0; // is_printable_ascii
        if (0x00 <= c && c <= 0x7f) n=0; // 0bbbbbbb
        else if ((c & 0xE0) == 0xC0) n=1; // 110bbbbb
        else if ( c==0xed && i<(ix-1) && ((unsigned char)string[i+1] & 0xa0)==0xa0) return false; //U+d800 to U+dfff
        else if ((c & 0xF0) == 0xE0) n=2; // 1110bbbb
        else if ((c & 0xF8) == 0xF0) n=3; // 11110bbb
        //else if (($c & 0xFC) == 0xF8) n=4; // 111110bb //byte 5, unnecessary in 4 byte UTF-8
        //else if (($c & 0xFE) == 0xFC) n=5; // 1111110b //byte 6, unnecessary in 4 byte UTF-8
        else return false;
        for (j=0; j<n && i<ix; j++) { // n bytes matching 10bbbbbb follow ?
            if ((++i == ix) || (( (unsigned char)string[i] & 0xC0) != 0x80))
                return false;
        }
    }
    return true;
}
*/

// ... réciproquement la chaine est-elle purement ASCII (ie sur 7bits) ?
//
bool charUtils::isPureASCII(const char* str)
{
    if (!IS_EMPTY(str)){
        const unsigned char* car = (const unsigned char*)str;
        while (*car != 0x00) {
             if (*car > 127){
                // Un seul caractère étendu suffit !!!
                return false;
            }

            // Caractère suivant
            car++;
        }

    }

    // Oui ...
    return true;
}

// Accès à un caractère par son index
//      retourne l'index reèl dans la chaîne
//
size_t charUtils::utf8_realIndex(const std::string& source, size_t index)
{
    size_t maxIndex(source.length());

    // Index trop grand => on retourne le dernier car.
    size_t searchIndex(maxIndex<=index?maxIndex-1:index);

#ifdef _WIN32
    // en ASCII pas de changement d'index
    return searchIndex;
#else

    // Parcours de la chaine
    size_t curIndex(0);
    const unsigned char* car = (const unsigned char*)source.c_str();
    while (*car != 0x00 && searchIndex) {
         if (*car > 127){
            // On saute le 2nd car.
            curIndex++;
            car++;
        }

        // Caractère suivant
        curIndex++;
        car++;
        searchIndex--;
    }

    return curIndex;
#endif // _WIN32
}

//
// base64
//

// Encodage base64
//	3 octets sources => 4 octets "base64"
//
std::string charUtils::toBase64(unsigned char const* bytes_to_encode, size_t in_len)
{
	std::string ret;
	int i = 0, j = 0;
	unsigned char char_array_3[3], char_array_4[4];

	while (in_len--){
		char_array_3[i++] = *(bytes_to_encode++);
		if (i == 3){
			char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
			char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
			char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
			char_array_4[3] = char_array_3[2] & 0x3f;

			for (i = 0; (i < 4); i++) {
				ret += base64_chars[char_array_4[i]];
			}
			i = 0;
		}
	}

	if (i)
	{
		for (j = i; j < 3; j++) {
			char_array_3[j] = '\0';
		}

		char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
		char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
		char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
		char_array_4[3] = char_array_3[2] & 0x3f;

		for (j = 0; (j < i + 1); j++) {
			ret += base64_chars[char_array_4[j]];
		}

		// On complète à 4 par le car. "="
		while ((i++ < 3)){
			ret += '=';
		}
	}

	return ret;
}

// Décodage base64
//
std::string charUtils::fromBase64(std::string const& encoded_string)
{
	size_t in_len = encoded_string.size();
	int i = 0, j = 0, in_ = 0;
	unsigned char char_array_4[4], char_array_3[3];
	std::string ret;

	while (in_len-- && (encoded_string[in_] != '=') && _is_base64(encoded_string[in_])){
		char_array_4[i++] = encoded_string[in_]; in_++;
		if (i == 4) {
			for (i = 0; i < 4; i++) {
				char_array_4[i] = (char)base64_chars.find(char_array_4[i]);
			}

			char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
			char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
			char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

			for (i = 0; (i < 3); i++) {
				ret += char_array_3[i];
			}
			i = 0;
		}
	}

	if (i)
	{
		for (j = i; j < 4; j++) {
			char_array_4[j] = 0;
		}

		for (j = 0; j < 4; j++) {
			char_array_4[j] = (char)base64_chars.find(char_array_4[j]);
		}

		char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
		char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
		char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

		for (j = 0; (j < i - 1); j++) {
			ret += char_array_3[j];
		}
	}

	return ret;
}

// Encodage complet d'une chaine de caractères
//
std::string charUtils::text2Base64(const std::string& source)
{
	std::string out("");
	size_t pos(0), tailleMax(source.size()), taille;
	const char* value = source.c_str();

	// Découpage ligne à ligne
	while (pos < tailleMax){
		taille = ((pos + RFC5322_BASE64_MAX_LINE_LEN)<tailleMax ? RFC5322_BASE64_MAX_LINE_LEN : (tailleMax - pos));
		out += toBase64(reinterpret_cast<const unsigned char*>(value + pos), taille) + "\r\n";
		pos += taille;
	}

	return out;
}

// Encodage complet du contenu d'un fichier
//
std::string charUtils::file2Base64(const std::string& fileName)
{
	std::string fContent("");

	// Ouverture du fichier
	FILE* file(NULL);
	if (NULL != (file = fopen(fileName.c_str(), "rb"))){
		size_t tailleMax(0), len(0);

		// Taille du fichier
		fseek(file, 0, SEEK_END);
		tailleMax = ftell(file);
		fseek(file, 0, SEEK_SET);

		size_t pointeur(0);
		char wBuffer[RFC5322_BASE64_MAX_LINE_LEN +1];		// Mon wBuffer de lecture
		while (pointeur < tailleMax){
			// Lecture d'un paquet
			len = fread(wBuffer, sizeof(char), RFC5322_BASE64_MAX_LINE_LEN, file);
			wBuffer[len] = 0;

			// Ajout de la ligne
			fContent += toBase64(reinterpret_cast<unsigned char*>(wBuffer), len) + "\r\n";

			pointeur += len;
		}

		// Fermeture du fichier
		fclose(file);
	}

	// Terminé
	return fContent;
}

// Encodage UTF8 & ("Quoted" ou "Base64")
//	Utilisé pour les mails
//
std::string charUtils::toMIMEUTF8(const std::string& source, const MIME_ENCODE mEncode)
{
	std::string out(""), in(source);

	// Quelque chose a été fait ?
	if (convert_toUTF8(in, true)){
		// Oui => on génère la chaine complète
		//
		if (mEncode == MIME_ENCODE::BASE64){
			out = "=?UTF-8?B?" + toBase64((const unsigned char*)source.c_str(), source.size()) + "?=";
		}
		else{
			out = "=?UTF-8?Q?" + in + "?=";
		}
	}
	else{
		// Non => doit-on mettre des doubles quotes ?
		bool quoted(false);
		const char* chaine = in.c_str();
		while (!quoted && *chaine){
			quoted = (!isalnum((unsigned char)*chaine) && NULL == strchr(RFC5322_ATOM, *chaine));
			chaine++;
		}

		if (quoted){
			out = "\"" + in + "\"";
		}
		else{
			out = in;	// On l'ajoute telle quelle
		}
	}

	return out;
}

// Création d'une "ligne" de type cle: valeur éventuellement sur plusieurs lignes
//
std::string charUtils::createKeyValueLine(const char* token, const std::string& value, size_t maxLineLength, const std::string& cEol)
{
	size_t len;
	if (!token ||
		!maxLineLength ||
		0 == (len = strlen(token)) ||
		!cEol.size()){
		return value;
	}

	std::string line(value);
	size_t curLen(maxLineLength - len - 2);		// On tient compte du nom et des 2 séparateurs
	size_t lineLen(line.size());
	size_t prev(0), next(0);
	size_t start(0), lstart(0);
	bool cont(true);

	// Découpage de la ligne
	while ((start + curLen) <= lineLen){
		lstart = start;
		prev = line.npos;
		cont = true;

		// Recherche de la position la plus proche de la fin de ligne
		// pour y inséer un saut de ligne (si nécessaire)
		while (cont){
			if (line.npos == (next = line.find(", ", lstart))){
				// Peut-être un espace
				next = line.find(" ", lstart);
			}

			// Suis je encore sur cette ligne ?
			if (false == (cont = (next < (start + curLen)))){
				// La coupure se fera
				//  - soit sur l'espace précédent
				//  - soit à la position actuelle

				if (next != line.npos && prev == line.npos){
					prev = next;
				}
			}
			else{
				// oui => on peut chercher le suivant
				prev = next;
				lstart = next + 1;
			}
		}

		// Dois je couper ?
		if (prev != line.npos){
			line.insert(prev, cEol);
			start = prev + 2 + cEol.size();			// On avance ...
		}
		else{
			// plus rien à faire
			start = lineLen;
		}

		curLen = maxLineLength;
	}

	// Création de la ligne complète
	std::string full(token);
	full += ": " + line + eol();
	return full;
}

// Découpe d'une ligne en fonction de sa longueur
//
std::string charUtils::splitLine(const std::string& source, size_t maxLineLength, const std::string& cEol)
{
	size_t end(0);
	if (0 == (end = source.size()) ||
		end < maxLineLength){
		return source;
	}

	std::string sLine = (cEol.size() ? cEol : eol_);
	sLine += " ";

	std::string line(source);
	size_t pos(0);

	// On coupe ...
	while ((pos + maxLineLength) < line.size()){
		// Insertion d'un saut de ligne
		pos += maxLineLength;
		line.insert(pos, sLine);

		pos+=(sLine.size() - 1);			// On avance ...
	}

	return line;
}


//
// Utilitaires
//

// Retrait des accents
//
std::string charUtils::removeAccents(const char* source)
{
	if (IS_EMPTY(source)){
		return "";
	}

#ifdef _WIN32
	std::string strAccents("ÀÁÂÃÄÅàáâãäåÒÓÔÕÖØòóôõöøÈÉÊËèéêëÇçÌÍÎÏìíîïÙÚÛÜùúûüÿÑñ");
	std::string strSansAccents("AAAAAAaaaaaaOOOOOOooooooEEEEeeeeCcIIIIiiiiUUUUuuuuyNn");

	std::string final("");
	size_t pos(0);
	char caractere;
	for (const char* car = source; (*car); car++){
		// (*car est le caractère courant)
		caractere = (*car);
		if (strAccents.npos == (pos = strAccents.find(caractere))){
			final += (*car);
		}
		else{
			// Remplacement par l'équivalent sans accent
			final += strSansAccents.substr(pos, 1);
		}
	}

	return final;
#else
    std::string text(source);
    return source;
#endif // _WIN32
}


// Comparaison sans accents
//
bool charUtils::equalWithoutAccents(std::string& left, const char *right)
{
	std::string cleanLeft = removeAccents(left);
	std::string cleanRight = removeAccents(right);

	// Comparaison
	return (cleanLeft == cleanRight);
}

// Nombre d'occurence d'une chaine dans une autre, en partant de ...
//
size_t charUtils::countOf(std::string& str, char searched, size_t from)
{
	size_t count(0);

	// Déja à la fin ?
	if (str.size() == from){
		return 0;
	}

	size_t pos(str.size() - from - 1);
	while (str.npos != (pos = str.rfind(searched, pos))){
		// Une occurence ...
		count++;

		// On recherche la suivante
		pos--;
	}

	return count;
}

// Transformation de la chaine en majuscules avec ou sans accents
//
std::string charUtils::upString(std::string& source, bool removeAccents)
{
	std::string sUpper = charUtils::strupr(source.c_str());
	return (removeAccents?charUtils::removeAccents(sUpper):sUpper);
}

// Conversion en majuscules
//
std::string charUtils::strupr(const char* source)
{
	std::string out("");
	const char* car = source;
	while (*car){
		out += toupper((unsigned char)*car);
		car++;
	}

	return out;
}

char* charUtils::strupr(char* source)
{
	if (!IS_EMPTY(source)){
        char* car = source;
        for (; *car; ++car){
            *car = toupper((unsigned char)*car);
            car++;
        }
    }

	return source;
}

// Conversion en minuscules
//
std::string charUtils::strlwr(const char* source)
{
	std::string out("");
	if (!IS_EMPTY(source)){
        const char* car = source;
        while (*car){
            out+=tolower((unsigned char)*car);
            car++;
        }
    }

	return out;
}

char* charUtils::strlwr(char* source)
{
	if (!IS_EMPTY(source)){
        char* car(source);
        for (; *car; ++car){
            *car = tolower((unsigned char)*car);
            car++;
        }
    }

	return source;
}

// Nettoyage d'une chaine
//
std::string charUtils::clean(const std::string& source, const char* toremove)
{
	if (NULL == toremove || 0 == strlen(toremove)){
		return source;
	}

	std::string out("");
	char* current = (char*)source.c_str();
	while (*current){
		if (NULL == strchr(toremove, *current)){
			out += *current;
		}

		// On avance
		current++;
	}

	return out;
}

// Comparaison de deux chaines
//
int charUtils::stricmp(const char* left, const char* right)
{
#ifdef _WIN32
	return ::_stricmp(left, right);
#else
	// Conversion en minuscules
	std::string sLeft = strlwr(left);
	std::string sRight = strlwr(right);

	// Comparaison
	return strcmp(sLeft.c_str(), sRight.c_str());
#endif // _WIN32
}

// Conversion
//
char* charUtils::itoa(int num, char* wBuffer, int radix, int digits)
{
	if (10 == radix) {
		std::string format("%");
		if (digits && digits < 10) {
			char dig('0');
			format += dig;
			dig += digits;
			format += dig;
		}
		format += "d";
		sprintf(wBuffer, format.c_str(), num);
	}
	else {
		// Pas en base 10 => pas de formatage
#ifdef _WIN32
		return _itoa(num, wBuffer, radix);
#else
		// itoa n'est pas définie par le cpp juste par certains compilateurs (dont vc++)
		sprintf(wBuffer, radix == 10 ? "%d" : "%X", num);
#endif // _WIN32
	}

	// Terminé
	return wBuffer;
}

std::string charUtils::itoa(int num, int radix, int digits)
{
	char buffer[20];
	charUtils::itoa(num, buffer, radix, digits);
	return std::string(buffer);
}

// Méthodes de conversion
//

// => UTF8
//
std::string charUtils::_ISO8859_1toUTF8(std::string &str)
{
	std::string out("");
	for (std::string::iterator it = str.begin(); it != str.end(); ++it){
		uint8_t ch = *it;
#ifdef _DEBUG
		if (ch == 140) {
			int i(3);
			i++;
		}
#endif // #ifdef _DEBUG
		if (ch < 0x80) {
			out.push_back(ch);
		}
		else{
			// http://html-codes.info/ansi/html/Windows-1252-Latin%20capital%20ligature%20OE_140

			//if (ch == 'Œ')
			if (ch == 140){
				/*
				if (extendedLatin_)
				{
				*/
				out.push_back((__int8)0xc5);
				out.push_back((__int8)0x92);
				/*
				}
				else
				{
					out.push_back((__int8)'O');
					out.push_back((__int8)'E');
				}
				*/
			}
			else{
				//if (ch == 'œ')
				if (ch == 156){
					/*
					if (extendedLatin_)
					{*/
					out.push_back((__int8)0xc5);
					out.push_back((__int8)0x93);
					/*}
					else
					{
						out.push_back((__int8)'o');
						out.push_back((__int8)'e');
					}*/
				}
				else{
					// Encodage standard
					out.push_back(0xc0 | ch >> 6);
					out.push_back(0x80 | (ch & 0x3f));
				}
			}
		}
	}

	return out;
}

// UTF8 =>
//
std::string charUtils::_UTF8toISO8859_1(const std::string& str)
{
	std::string out("");
	if (!str.size()){
		return out;
	}

	unsigned int codepoint(0);
	const char* in = str.c_str(), *prev(NULL);
	while (*in != 0){
		unsigned char ch = static_cast<unsigned char>(*in);
		if (ch <= 0x7f)
			codepoint = ch;
		else if (ch <= 0xbf)
			codepoint = (codepoint << 6) | (ch & 0x3f);
		else if (ch <= 0xdf)
			codepoint = ch & 0x1f;
		else if (ch <= 0xef)
			codepoint = ch & 0x0f;
		else
			codepoint = ch & 0x07;
		prev = (in++);
		if (((*in & 0xc0) != 0x80) && (codepoint <= 0x10ffff)){
			if (codepoint <= 255){
				out.append(1, static_cast<char>(codepoint));
			}
			else{
				// Autres caractères
				//
				if (*(prev-1) == (__int8)0xc5){
					if (*prev == (__int8)0x92){
						out.push_back((__int8)140);
					}
					else{
						if (*prev == (__int8)0x93){
							out.push_back((__int8)156);
						}
					}
				}
			}
		}
	}
	return out;
}

// Version WIN32
//

#ifdef _WIN32
// Conversion en/de UTF8
//
//	Dans tous les cas, on convertit en 2 temps :
//		- source -> WCHAR
//		- WCHAR -> destination
//

std::string charUtils::_win32_UTF8Convert(const std::string& str, UINT from, UINT to)
{
	// Rien à faire ...
	if (0 == str.length() ||
		from == to) {
		return str;
	}

	// Dans un premier temps on convertit en WCHAR
	//
	wchar_t* wBuffer(NULL);
	size_t size(1 + str.length());
	int wBuffLen((int)(sizeof(wchar_t) * size));
	if (NULL != (wBuffer = (wchar_t*)malloc(wBuffLen))) {
		if (0 == MultiByteToWideChar(from, 0, (LPCCH)str.c_str(), -1, wBuffer, wBuffLen)) {
			// Erreur lors de la conversion
			return "";
		}
	}
	else {
		// Erreur d'allocation
		// On pourrait peut-être lever une exception ....
		return "";
	}

	// Puis on convertit de WCHAR vers le format demandé
	//

	// Le buffer destination aura la même taille que celle de la source (WCHAR)
	// c'est trop, c'est certain, mais qui peut le plus peut le moins !
	char* destBuffer(NULL);
	bool valid(false);
	std::string out("");

	if (NULL != (destBuffer = (char*)malloc(sizeof(char) * wBuffLen)) &&
		0 != WideCharToMultiByte(to, 0, wBuffer, -1, destBuffer, wBuffLen, NULL, NULL)) {
		valid = true;
	}

	// Libérations
	if (valid) {
		out = destBuffer;
		free(destBuffer);
		free(wBuffer);
	}

	return out;
}
#endif // #ifdef _WIN32

// Taille => on tronque la chaine
//
std::string charUtils::shorten(const std::string& source, const size_t max)
{
	size_t len = source.size();
	if (len <= max) {
		return source;
	}

	// Nb de car. à prendre de la gauche
	size_t leftIndex(utf8_realIndex(source, (max - 3) / 2));

	// Index du 2nd pointeur, pour la partie droite
	size_t rightIndex(utf8_realIndex(source, len - max + leftIndex + 2));

	std::string newStr(source.substr(0, leftIndex));
	newStr += "...";
	newStr += source.substr(rightIndex);

	return newStr;
}

// EOF
