//--------------------------------------------------------------------------------
//--
//--	Fichier		:	RegEntry.h
//--
//--
//--
//--	Rôle		:	Définition de la classe CRegEntry & des classes associées
//--					
//--					Gestion de la base de registre
//--					
//--					Cette classe permet d'écrire et lire des données "n'importe ou"
//--					dans la base de registre
//--					
//--					La logique utilisée est celle des fichiers "INI" :
//--					
//--						- On dispose d'un repertoire de base ( BaseDirectory)
//--					
//--						- On y crée des sections dans lesquelles sont enregsitrées
//--							les différentes valeurs
//--					
//--					
//--					ex : [HKEY_CURRENT_USER\Software\Matra-com\DECT\monApplication]
//--					
//--						|
//--						|
//--						--- [Section 1]
//--						|			|
//--						|			--- [Entrée 1] = Valeur
//--						|			|
//--						...			...
//--						|			|
//--						|			--- [Entrée n] = Valeur
//--					
//--						...
//--						--- [Section n]
//--						...
//--					
//--
//--	Auteur		:	Jérôme Henry-Barnaudière
//--
//--	Creation	:	Juillet 1998
//--
//--	Version		:	3.0.1
//--
//--	Date		:	Février 2003
//--
//--------------------------------------------------------------------------------

#ifndef _REG_ENTRY_H_
#define _REG_ENTRY_H_

#define _REG_ENTRY_VER_		0x0301		/* version courante de regentry */

//#define REG_COPY_KEY

//
//	Utilitaires
//
#ifndef IS_EMPTY
#include <tchar.h>
#define IS_EMPTY(szIn)	(!(szIn) || (0 == _tcslen((szIn))))
#endif	//	#ifndef IS_EMPTY

//-----------------------------------------------------------------------------------------------------
//--
//--	Directives de compilation => reduction des sources
//--
//-----------------------------------------------------------------------------------------------------

#define _USES_REG_ENUMS_
#define _USES_REG_DELETE_

//-------------------------------------------------------------------------------
//--	
//--	CONSTANTES
//--	
//-------------------------------------------------------------------------------

#define CLASSES_ROOT			HKEY_CLASSES_ROOT 
#define CURRENT_USER			0x80000001
#define LOCAL_MACHINE			0x80000002
#define USERS					0x80000003
#define PERFORMANCE_DATA		0x80000004
#if(WINVER >= 0x0400)
#define CURRENT_CONFIG			0x80000005
#define DYN_DATA				0x80000006
#endif

//
// Chaînes utilisées pour les booléens
//
#ifndef STR_YES
#define STR_YES		_T("yes")
#define STR_NO		_T("no")
#endif	//	#ifndef STR_YES


//
//	Chaîne vide pour un champs existant : [champs] = []
//
#ifndef EMPTY_STRING
#define	EMPTY_STRING			_T("__empty___")
#endif //#ifndef EMPTY_STRING

#include <deque>			/* les versions >= 3 utilisent les STL */
#include <string>
using namespace std;

//
//	CRegKeyList <= Enumération des clefs
//
class CRegKeyList
{
public:
	
	// Construction & destruction
	//
	CRegKeyList()
	{}
	
	~CRegKeyList()
		{ FreeList(); }

	void FreeList();
	
	// Ajout
	//
	BOOL AddKeyName(LPTSTR szNew);

	// Consultation
	//
	size_t GetKeyCount()
		{ return _list.size(); }
	size_t size()
		{ return GetKeyCount(); }

	size_t GetKeyName(DWORD dwIndex, LPTSTR szDest, size_t cbDest);
	LPCTSTR GetKeyName(size_t index)
		{ return (index>=_list.size()?NULL:_list[index]); }
	

private:
	deque<LPTSTR>	_list;
};

//
// Valeurs
//
union REGVALUE
{
	int		iValue;
	BOOL	bValue;
	LPTSTR	szValue;
	LPVOID	pValue;
};

//
//	CRegValueList <= Enumération des valeurs d'une clef
//
class CRegValueList
{
public:
	
	enum regValueType {VALUE_INT, VALUE_BOOL, VALUE_STRING, VALUE_BINARY};
	
	typedef struct _tagKEYVALUE
	{
		regValueType	type;
		LPTSTR			szValueName;
		DWORD			dwTypeValue;
		DWORD			dwValueSize;
		REGVALUE		Value;
	}KEYVALUE, * LPKEYVALUE;
	
	// Construction & destruction
	//
	CRegValueList() {}
	~CRegValueList()
	{ FreeList(); }

	VOID FreeList();
	
	// Ajouts
	//
	BOOL AddValueString(LPTSTR szName,LPCTSTR szValue);
	BOOL AddValueInt(LPTSTR szName,int iValue, regValueType type = VALUE_INT);
	BOOL AddValueBOOL(LPTSTR szName,BOOL bValue)
	{ return AddValueInt(szName,bValue,VALUE_BOOL);}
	BOOL AddValueBinary(LPTSTR szName,LPVOID pValue, DWORD dwTypeValue, DWORD dwSize);
	
	// Consultation
	//
	size_t GetValueCount()
		{ return _list.size(); }
	size_t size()
		{ return GetValueCount(); }

	const LPKEYVALUE GetValue(size_t index);
	

private:
	BOOL AddValue(LPTSTR szName,LPKEYVALUE);
	
	deque<LPKEYVALUE>	_list;
};

//
//	CRegEntry <= Gestion de la registry
//

class CRegEntry
{
// Méthodes publiques
//
public:

	// Construction / Destruction
	//	
	CRegEntry()
		{ _hRootKey = NULL;}

	~CRegEntry()
		{ _init();}

	// suis je valide ?
	//
	BOOL IsValid()
		{ return (NULL != _hRootKey);}

	// Assignation du repertoire de base
	//
	//	Params	:	Racine ( ex : HKEY_CURRENT_USER )
	//
	//				Chemin de base à partir de la racine
	//					(ex :  "Software\\Mirabelle\\")
	BOOL SetBaseDirectory(HKEY hFrom, LPCTSTR szFolder);
	
	
	// Gestion de l'arborescence
	// 
	BOOL EmptyDirectory();
	VOID CreateKey(HKEY parent, LPCTSTR path)
		{ CreeClef(parent, path);}
	VOID DeleteKey(LPCTSTR szKey);

	BOOL MoveTo(LPCTSTR szSection);

	
	// Lecture / Ecriture des entiers
	//
	int GetPrivateInt(LPCTSTR szSection, LPCTSTR szEntry, int defValue);
	BOOL WritePrivateInt(LPCTSTR szSection, LPCTSTR szEntry, int iValue);

	// Lecture / Ecriture des chaînes de caractère
	//
	VOID GetPrivateString(LPCTSTR szSection, LPCTSTR szEntry, LPCTSTR szdefValue, LPTSTR szRetour, UINT lgRetour, bool expand = false);
	BOOL WritePrivateString(LPCTSTR szSection, LPCTSTR szEntry, LPCTSTR szIn);

	string GetPrivateString(LPCTSTR szSection, LPCTSTR szEntry, LPCTSTR szdefValue, bool expand = false);
	
	// Lecture / Ecriture des Booléens ("oui" / "non")
	//
	BOOL GetPrivateBOOL(LPCTSTR szSection, LPCTSTR szEntry, BOOL bDefValue);
	BOOL WritePrivateBOOL(LPCTSTR szSection, LPCTSTR szEntry, BOOL bValue);

	// Lecture / Ecriture d'une série d'octets (BINARIES)
	//
	DWORD GetPrivateBinary(LPCTSTR szSection, LPCTSTR szEntry, LPVOID pRetour, DWORD dwRetour, DWORD* dwType, BOOL* pbRet);
	BOOL WritePrivateBinary(LPCTSTR szSection, LPCTSTR szEntry, LPVOID pBuffer, DWORD dwBuffer, DWORD dwType = REG_BINARY);
		
	// Suppression d'une entrée
	//
	BOOL ClearEntries();
	VOID DeleteEntry(LPCTSTR szSection, LPCTSTR szEntry);

	//
	//	Enumérations
	//
	size_t EnumFolders(CRegKeyList* pEnumerator)
		{ return EnumKeys(pEnumerator); }
	size_t EnumKeys(CRegKeyList* pEnumerator);
	size_t EnumValues(CRegValueList* pEnumerator);
	size_t EnumValues(LPCTSTR szKey, CRegValueList* pEnumerator);

	// Copie d'une "branche" dans une autre
	//
#ifdef REG_COPY_KEY
	BOOL CopyKey(LPCTSTR szSrc, HKEY hDest, LPCTSTR szDestKey);
	BOOL CopyKey(LPCTSTR szSrc, HKEY hDest, LPCTSTR szDestRoot, LPCTSTR szDestKey);
#endif	//	#ifdef REG_COPY_KEY

	// Accès
	//
	HKEY GetHandle()
		{ return _hRootKey;}

	void Attach(HKEY hHandle)
		{ _init(); _hRootKey = hHandle; }

	void Detach()
		{ Attach(NULL); }


// Méthodes privées
//
private :

	VOID _init();

	VOID CloseKey(HKEY hKey)
	{
		if (hKey != _hRootKey)
		{
			RegCloseKey(hKey);
		}
	}

	// Création d'une nouvelle clef
	//
	static HKEY PASCAL CreeClef(HKEY hMum, LPCTSTR szPath);
	static HKEY CreeClef(HKEY hMum, LPCTSTR szPath, DWORD dOpeningPrivilege);

	// Ouverture d'une clef
	//
	static HKEY PASCAL _OuvreClef(HKEY hMum, LPCTSTR szPath);
	
	// Supression recursive d'une arborescence
	//
	//		Moteur appelé par DeleteDirectory
	//
	VOID PASCAL DeleteSubKeys(HKEY hMum);

	// Moteurs d'enumération
	//
	size_t _EnumKeys(HKEY hMum, CRegKeyList* pEnumerator);
	size_t _EnumValues(HKEY hMum, CRegValueList* pEnumerator);


// Données membres
//
private :
	// Pointeur sur le répertoire de base
	HKEY _hRootKey;
};



#endif	//	#ifndef _REG_ENTRY_H_

// EOF