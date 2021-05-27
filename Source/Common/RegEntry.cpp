//--------------------------------------------------------------------------------
//--
//--	Fichier		:	RegEntry.cpp
//--
//--
//--
//--	Rôle		:	Implémentation de la classe CRegEntry & des classes associées
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

#ifdef __AFX_H__
#include "stdafx.h"
#else
#include "commonTypes.h"
#endif // #ifdef __AFX_H__

#include <shlwapi.h>

#include "RegEntry.h"


//--------------------------------------------------------------------------------
//--
//--	CRegEntry
//--
//--------------------------------------------------------------------------------


//--------------------------------------------------------------------------------
//--
//--	ENTIERS
//--
//--------------------------------------------------------------------------------

// Lecture d'un entier
//
//	params	:	Nom de la section
//				Valeur recherchée
//				Valeur par défaut (si absente...)
//
//	retour	:	Valeur lue
//
int CRegEntry::GetPrivateInt(LPCTSTR szSection, LPCTSTR szEntry, int iDefValue)
{
	LONG erreur;
	UINT retour;
	DWORD dwType ;	// type de données
	DWORD dwTaille = sizeof(UINT);
	
	// Pas fou !!!
	//
	if (!_hRootKey)
		return 0;

	// Pointeur sur la mère
	//
	HKEY hMum = CreeClef(_hRootKey,szSection);
	
	// Lecture de la valeur
	//
	erreur = RegQueryValueEx(
		hMum,
		szEntry,
		NULL,
		&dwType,
		(LPBYTE) &retour,
		&dwTaille
		);

	// On verifie que la valeur existe bien
	//
	if (erreur != ERROR_SUCCESS || dwType != REG_DWORD)
		retour = iDefValue;

	// Libération
	//
	CloseKey(hMum);
	
	return retour;
}


// Ecriture d'un entier
//
//	params	:	Nom de la section
//				Valeur a ecrire
//				Valeur 
//
BOOL CRegEntry::WritePrivateInt(LPCTSTR szSection, LPCTSTR szEntry, int iValue)
{
	LONG erreur;
	
	if (!_hRootKey)
	{
		return FALSE;
	}

	// Pointeur sur la mère
	//
	HKEY hMum = CreeClef(_hRootKey,szSection);

	// Ecriture
	//
	erreur = RegSetValueEx(
		hMum,
		szEntry,
		0,
		REG_DWORD,
		(LPBYTE) &iValue,
		sizeof(UINT)
		);

	// Libération
	//
	CloseKey(hMum);

	return TRUE;
}

//--------------------------------------------------------------------------------
//--
//--	Chaînes de caractères
//--
//--------------------------------------------------------------------------------

// Lecture d'une chaîne
//
//	params	:	Nom de la section
//				Valeur recherchée
//				Valeur par défaut (si absente...)
//				Pointeur sur la chaîne lue
//				Longueur du buffer
//
VOID CRegEntry::GetPrivateString(LPCTSTR szSection, LPCTSTR szEntry, LPCTSTR strdefValue, LPTSTR strRetour, UINT lgRetour, bool expand)
{
	LONG erreur;
	DWORD dwType ;	// type de données
	DWORD dwTaille = lgRetour;

	if (!_hRootKey)
		return;

	// Pointeur sur la mère
	//
	HKEY hMum = CreeClef(_hRootKey,szSection);

	// Lecture
	//
	erreur = RegQueryValueEx(
		hMum,
		szEntry,
		NULL,
		&dwType,
		(LPBYTE)strRetour,
		&dwTaille
		);

	// On verifie que la valeur existe bien
	if (erreur == ERROR_SUCCESS && ((REG_SZ == dwType) || (REG_EXPAND_SZ  == dwType)))
	{
		// Valeur à étendre ?
		if (REG_EXPAND_SZ  == dwType && dwTaille && expand)
		{
			TCHAR inter[MAX_PATH+1];
			ExpandEnvironmentStrings(strRetour, inter, MAX_PATH);
			lstrcpy(strRetour, inter);
		}
	}
	else
	{
		// Erreur => valeur par défaut
		lstrcpy((LPTSTR)strRetour,strdefValue);
	}

	// Libération
	//
	CloseKey(hMum);
}

// Ecriture d'une chaîne
//
//	params	:	Nom de la section
//				Valeur a ecrire
//				Valeur 
//
BOOL CRegEntry::WritePrivateString(LPCTSTR szSection, LPCTSTR szEntry, LPCTSTR strIn)
{
	/*
	// Vide => rien à faire ...
	//
	if (!strIn || !strIn[0] || !_hRootKey)
	{
		return FALSE;
	}
	*/

	// Pointeur sur la mère
	//
	HKEY hMum = CreeClef(_hRootKey, szSection);

	LONG erreur;

	// Vide ?
	//
	//if (!strIn || !_tcslen(strIn) || !_tcscmp(strIn, EMPTY_STRING) || !strIn[0])
	if (IS_EMPTY(strIn) || !_tcscmp(strIn, EMPTY_STRING))	
	{
		// Ecriture
		//
		erreur = RegSetValueEx(
			hMum,
			szEntry,
			0,
			REG_SZ,
			(LPBYTE)_T(" \0"),
			sizeof(TCHAR));
	}
	else
	{
		// Ecriture
		//
		erreur = RegSetValueEx(
			hMum,
			szEntry,
			0,
			REG_SZ,
			(LPBYTE)strIn,
			(DWORD)(_tcslen(strIn)+1)*sizeof(TCHAR));
	}

	// Libération
	//
	CloseKey(hMum);

	return TRUE;
}


string CRegEntry::GetPrivateString(LPCTSTR szSection, LPCTSTR szEntry, LPCTSTR szdefValue, bool expand)
{
	//TCHAR szInter[MAX_PATH+1];
	TCHAR szInter[2048];
	GetPrivateString(szSection, szEntry, szdefValue,szInter,2048, expand);
	return szInter;
}

//--------------------------------------------------------------------------------
//--
//--	BOOLEENS
//--	
//--			Les booléens sont enregistrés sous forme de chaînes ("oui"/"non")
//--
//--------------------------------------------------------------------------------


BOOL CRegEntry::WritePrivateBOOL(LPCTSTR szSection, LPCTSTR szEntry, BOOL bValue)
{
	// Ecriture de la chaîne de caractères
	//
	return WritePrivateString(szSection,szEntry,bValue?STR_YES:STR_NO);	
}

BOOL CRegEntry::GetPrivateBOOL(LPCTSTR szSection, LPCTSTR szEntry, BOOL bDefValue)
{
	TCHAR strLue[10];

	// Lecture de la chaîne
	//
	GetPrivateString(szSection,szEntry,bDefValue?STR_YES:STR_NO,strLue,9);

	// Comparaison avec STR_YES
	//
	return (_tcscmp(strLue,STR_YES) == 0);
}

//--------------------------------------------------------------------------------
//--
//--	BINARIES
//--
//--------------------------------------------------------------------------------

DWORD CRegEntry::GetPrivateBinary(LPCTSTR szSection, LPCTSTR szEntry, LPVOID pRetour, DWORD dwRetour, DWORD* dwType, BOOL* pbRet)
{
	// Par défaut je plante !!!
	//
	(*pbRet) = FALSE;
	
	LONG erreur;
	DWORD dwTaille = dwRetour;
	
	if (!_hRootKey || !pRetour || !dwRetour || !dwType)
	{
		return 0;
	}

	(*dwType) = REG_BINARY;		// type de données

	// Pointeur sur la mère
	//
	HKEY hMum = CreeClef(_hRootKey,szSection);

	// Lecture
	//
	erreur = RegQueryValueEx(
		hMum,
		szEntry,
		NULL,
		dwType,
		(LPBYTE)pRetour,
		&dwTaille
		);

	
	if (erreur == ERROR_SUCCESS && ((*dwType) != REG_SZ) && ((*dwType) != REG_DWORD))
	{
		(*pbRet) = TRUE;
	}
	
	
	// Fin ...
	//
	CloseKey(hMum);
	
	//
	// bRet = TRUE		=> Retour = Nbre d'occtets effectivements lus ...
	//
	// bRet = FALSE		=> Retour = Nbre d'occtets necessaires (le buffer doit être trop petit)
	//
	return dwTaille;
}

BOOL CRegEntry::WritePrivateBinary(LPCTSTR szSection, LPCTSTR szEntry, LPVOID pBuffer, DWORD dwBuffer, DWORD dwType)
{
	// Vide => rien à faire ...
	//
	if (!pBuffer || !dwBuffer || !_hRootKey)
	{
		return FALSE;
	}
	
	LONG erreur;
	
	// Pointeur sur la mère
	//
	HKEY hMum = CreeClef(_hRootKey,szSection);

	// Ecriture
	//
	erreur = RegSetValueEx(
		hMum,
		szEntry,
		0,
		dwType,
		(LPBYTE)pBuffer,
		dwBuffer);

	// Libération
	//
	CloseKey(hMum);

	return TRUE;
}

//--------------------------------------------------------------------------------
//--
//--	UTILITAIRES
//--
//--------------------------------------------------------------------------------

VOID CRegEntry::_init()
{ 
	try
	{
		if (_hRootKey)
		{
		
			RegCloseKey(_hRootKey);			
		}
		
		_hRootKey = NULL;
	}
	catch (...)
	{
	}
}


// Fixer le repertoire de base
//
//	Params	:	Racine ( ex : HKEY_CURRENT_USER )
//				Chemin de base à partir de la racine
//					(ex :  "Software\\Mirabelle\\")
//
//	retour	:	Opération effectuée ?
//
BOOL CRegEntry::SetBaseDirectory(HKEY hFrom, LPCTSTR szFolderIn)
{
	if ((!szFolderIn || !szFolderIn[0]) && !hFrom)
	{
		return FALSE;
	}

	// On ne sait jamais ...
	//
	_init();

	// Une racine !!!
	//
	if ((hFrom && !szFolderIn) ||
		(hFrom && !szFolderIn[0]))
	{
		_hRootKey = hFrom;
		return TRUE;
	}

	HKEY hKey = (HKEY)hFrom;
	HKEY hNew = NULL;
	TCHAR strClef[80];
	TCHAR *pChaine;

	//
	//	Il me faut un "\" à la fin
	//
	size_t len = _tcslen(szFolderIn);
	TCHAR *pRef = NULL;

	// Est ce nécessaire ?
	//
	if (szFolderIn[len-1] != _T('\\'))
	{

		pChaine = (TCHAR*)malloc((len+2)*sizeof(TCHAR));

		_tcscpy_s(pChaine, len+1, szFolderIn);
		pChaine[len] = _T('\\');
		pChaine[len+1] = _T('\0');
		pRef = pChaine;
	}
	else
	{
		pChaine = (TCHAR*)szFolderIn;
	}

		
	// Pointeur sur le premier "\"
	//
	TCHAR * pCourant = _tcschr(pChaine,'\\');

	// Decoupage des repertoire
	//	+ création des clefs intermédiaires
	//
	while (pCourant)
	{
		// Extraction de la clef
		//
		memset(strClef,0,80*sizeof(TCHAR));
		_tcsncpy_s(strClef, 79, pChaine, (pCourant-pChaine));

		// Création de la clef
		//
		if (NULL == (hNew = CreeClef(hKey,strClef)))
		{
			// Le chemin n'existe pas
			//
			if (pRef)
			{
				free(pRef);
			}
			
			return FALSE;
		}

		// Libération de l'ancienne clef
		//
		if (hKey != (HKEY)hFrom)
		{
			RegCloseKey(hKey);
		}

		hKey = hNew;

		// On avance
		//
		pChaine = pCourant+1;
		
		// Clef suivante...
		//
		pCourant = _tcschr(pChaine,'\\');
	}

	_hRootKey = hNew;

	// Ai je fait une copie ?
	//
	if (pRef)
	{
		free(pRef);
	}

	// OK
	return TRUE;
}

BOOL CRegEntry::MoveTo(LPCTSTR szSection)
{
	if (NULL == _hRootKey)
	{
		return FALSE;
	}

	HKEY hMum = CreeClef(_hRootKey,szSection);

	// not a leaf !!!
	if (NULL == hMum)
	{
		return FALSE;
	}

	// go to this "leaf"
	_hRootKey = hMum;
	return TRUE;
}

// Création d'une nouvelle clef
//
HKEY CRegEntry::CreeClef(HKEY hMum, LPCTSTR szPath)
{
	HKEY hRetour;

	// J'essaie d'abord de l'ouvrir avec tous les privilèges ...
	//
	hRetour = CreeClef(hMum,szPath,KEY_ALL_ACCESS | KEY_EXECUTE | KEY_WRITE | KEY_ENUMERATE_SUB_KEYS);
	
	// En ecriture
	//
	if (!hRetour)
		hRetour = CreeClef(hMum,szPath,KEY_READ | KEY_WRITE);
	
	// En lecture seule ...
	//
	if (!hRetour)
		return CreeClef(hMum,szPath,KEY_READ);
	return hRetour;
}


HKEY CRegEntry::CreeClef(HKEY hMum, LPCTSTR szPath, DWORD dOpeningPrivilege)
{
	if (!szPath)
	{
		return hMum;
	}
	
	HKEY hRetour;
	DWORD Disposition;
		
	RegCreateKeyEx(
		hMum,
		szPath,
		0,
		_T(""),
#ifndef UNDER_CE
		REG_OPTION_NON_VOLATILE, 
#else
		0,
#endif
		dOpeningPrivilege,
		NULL,
		&hRetour,
		&Disposition
		);

	return hRetour;
}

// Ouverture d'une clef existante
//
//	Retourne un handle sur le clef si elle existe (NULL sinon);
//
HKEY PASCAL CRegEntry::_OuvreClef(HKEY hMum, LPCTSTR szPath)
{
	HKEY hRetour;
	LONG result;
	
	result = 
		RegOpenKeyEx(
		hMum,
		szPath,
		0,
#ifndef UNDER_CE
		KEY_ALL_ACCESS | KEY_EXECUTE | KEY_WRITE | KEY_ENUMERATE_SUB_KEYS,
#else
		0,
#endif
		&hRetour
		);

	return (result == ERROR_SUCCESS)?hRetour:NULL;
}


// Suppression d'une entrée
//
VOID CRegEntry::DeleteEntry(LPCTSTR szSection, LPCTSTR szEntry)
{
	// Pointeur sur la mère
	//
	HKEY hMum = szSection?CreeClef(_hRootKey,szSection):_hRootKey;

	 RegDeleteValue(hMum,(LPCTSTR)szEntry);
	
	// Libération
	//
	//RegCloseKey(hMum);
}

// Suppression d'un repertoire
//
//	retour	:	Opération effectuée ?
//
BOOL CRegEntry::ClearEntries()
{
	if (!_hRootKey)
	{
		return FALSE;
	}
	
	CRegValueList Enum;
	EnumValues(&Enum);

	// Je parcours la liste, et je l'efface ...
	//
	size_t uCount = Enum.GetValueCount();
	for (size_t i=0;i<uCount;i++)
	{
		CRegValueList::LPKEYVALUE pValue = Enum.GetValue(i);

		if (pValue)
		{
			RegDeleteValue(_hRootKey,(LPCTSTR)pValue->szValueName);
		}		
	}

	return (0<uCount);
}

// Suppression d'un repertoire
//
//	retour	:	Opération effectuée ?
//

BOOL CRegEntry::EmptyDirectory()
{
	if (!_hRootKey)
		return FALSE;
	
	// Suppression de l'arborescence
	//
	DeleteSubKeys(_hRootKey);

	// Suppression de toutes les valeurs
	//
	ClearEntries();
	
	_init();
		
	// OK
	return TRUE;
}

// Suppression de tout une arborescence
//
VOID CRegEntry::DeleteKey(LPCTSTR szKey)
{	
	if (!_hRootKey ||!szKey) return;
	
	HKEY hRef = _hRootKey;
	
	// Je vide le repertoire
	//
	if (NULL != (_hRootKey = _OuvreClef(_hRootKey, szKey)))
	{
		EmptyDirectory();
		RegDeleteKey(hRef,szKey);		
	}

	// Retour à la normale !!!
	//
	_hRootKey = hRef;
}

// Suppression des sous-clefs à partir de hMum ( hMum compris)
//
VOID PASCAL CRegEntry::DeleteSubKeys(HKEY hMum)
{
	TCHAR	szSubKeyName[MAX_PATH+1];		
		
	CRegKeyList Enum;
	_EnumKeys(hMum,&Enum);

	// Je parcours la liste, et je l'efface ...
	//
	for (UINT i=0;i<Enum.GetKeyCount();i++)
	{
		Enum.GetKeyName(i,szSubKeyName,MAX_PATH);	

		// Pointeur sur la clef courante
		//
		HKEY hSubKey = CRegEntry::_OuvreClef(hMum,szSubKeyName);

		if (hSubKey)
		{
			// Destruction recursive des sous-clefs
			//
			DeleteSubKeys(hSubKey);
			
			// Destruction de la clef
			//
			RegCloseKey(hSubKey);
			RegDeleteKey(hMum,szSubKeyName);
		}
		
	}
}

//	Enumération des clefs
//
size_t CRegEntry::_EnumKeys(HKEY hMum, CRegKeyList* pEnumerator)
{
	// Petites verifications
	//
	if (!hMum || !pEnumerator)
	{
		return 0;
	}
	pEnumerator->FreeList();

	
	TCHAR		szSubKeyName[MAX_PATH];
		
	DWORD    dwcSubKeys;
	DWORD	   cbName;
	DWORD    dwcMaxSubKey, dwcMaxClass, dwcValues, dwcMaxValueName;
	DWORD    dwcMaxValueData, dwcSecDesc;
	FILETIME ftLastWriteTime;
	
	RegQueryInfoKey (hMum, NULL, 0, NULL, &dwcSubKeys, &dwcMaxSubKey, &dwcMaxClass,
		&dwcValues, &dwcMaxValueName, &dwcMaxValueData, &dwcSecDesc, &ftLastWriteTime);

	// Enumération des sous-clefs
	//
	for (DWORD i=0, retCode = ERROR_SUCCESS; retCode == ERROR_SUCCESS; i++)
	{
		cbName = MAX_PATH;

		retCode = RegEnumKeyEx( hMum, i, szSubKeyName, &cbName, NULL, NULL, NULL, NULL);

		// Une nouvelle valeur !!!
		//
		if (retCode == (DWORD)ERROR_SUCCESS)
		{
			pEnumerator->AddKeyName(szSubKeyName);
		}
	}

	return pEnumerator->GetKeyCount();
}

size_t CRegEntry::EnumKeys(CRegKeyList* pEnumerator)
{
	return _EnumKeys(_hRootKey,pEnumerator);
}

// Enumération des valeurs
//
size_t CRegEntry::EnumValues(CRegValueList* pEnumerator)
{
	return _EnumValues(_hRootKey,pEnumerator);
}

size_t CRegEntry::EnumValues(LPCTSTR szKey, CRegValueList* pEnumerator)
{
	HKEY hKey = _OuvreClef(_hRootKey, szKey);
	if (hKey)
	{
		DWORD ret = (DWORD)_EnumValues(hKey,pEnumerator);
		RegCloseKey(hKey);

		return (size_t)ret;
	}
	return 0;
}

size_t CRegEntry::_EnumValues(HKEY hMum, CRegValueList* pEnumerator)
{
	// Petites verifications
	//
	if (!hMum || !pEnumerator)
	{
		return 0;
	}
	pEnumerator->FreeList();

	DWORD    dwcSubKeys;
	DWORD    dwcMaxSubKey, dwcMaxClass, dwcValues, dwcMaxValueName;
	DWORD    dwcMaxValueData, dwcSecDesc;
	FILETIME ftLastWriteTime;
	
	RegQueryInfoKey (hMum, NULL, 0, NULL, &dwcSubKeys, &dwcMaxSubKey, &dwcMaxClass,
		&dwcValues, &dwcMaxValueName, &dwcMaxValueData, &dwcSecDesc, &ftLastWriteTime);

	// Enumération des Valeurs
	//
	TCHAR  ValueName[MAX_PATH];
	DWORD dwcValueName = MAX_PATH;
	LPVOID pBuffer;
	DWORD dwSize, dwType;

	// Allocation d'un buffer ayant la taille requise !!!
	//
	pBuffer = malloc(dwcMaxValueData);

	if (!pBuffer)
	{
		return 0;
	}

	if (dwcValues)
	{
		for (DWORD j = 0, retValue = ERROR_SUCCESS; j < dwcValues; j++)
		{
			dwcValueName = MAX_PATH;
			ValueName[0] = _T('\0');
			dwSize = dwcMaxValueData;
			
			retValue = RegEnumValue (_hRootKey, j, ValueName, &dwcValueName,
							 NULL,
							 &dwType,
							 (LPBYTE)pBuffer,
							 &dwSize);
			
			if (!lstrlen(ValueName))
			{
				lstrcpy (ValueName, _T("<NO NAME>"));
			}

			switch (dwType)
			{
			case REG_SZ:
				// La valeur peut être vide !!!
				//
				if (pBuffer && _tcslen((TCHAR*)pBuffer))
				{
					pEnumerator->AddValueString(ValueName,(TCHAR*)pBuffer);
				}
				else
				{
					pEnumerator->AddValueString(ValueName,EMPTY_STRING);
				}
				
				break;
			
			case REG_DWORD:
			{
				int iInter;

				memcpy(&iInter,pBuffer,sizeof(int));
				pEnumerator->AddValueInt(ValueName,iInter);
				break;
			}

			// C'est une valeur "binaire"
			//
			default :
			{
				pEnumerator->AddValueBinary(ValueName,pBuffer, dwType, dwSize);
				break;
			}
			}

		}
	}

	// Je n'ai plus besoin du buffer
	//
	free(pBuffer);

	return pEnumerator->GetValueCount();
}

// Copie d'une "branche" dans une autre
//
#ifdef REG_COPY_KEY
BOOL CRegEntry::CopyKey(LPCTSTR szSrc, HKEY hDest, LPCTSTR szDestKey)
{
#ifdef SHCopyKey
	CRegEntry destEntry;

	// Je verifie que tout est OK
	//
	if (!_hRootKey || IS_EMPTY(szSrc) || IS_EMPTY(szDestKey)
		|| FALSE == destEntry.SetBaseDirectory(hDest,szDestKey))
	{
		return FALSE;
	}

	// Si elle existe déja, je la retire !
	//
	destEntry.DeleteKey(szSrc);

	// OK
	//
	return (ERROR_SUCCESS == SHCopyKey(_hRootKey,szSrc,destEntry.GetHandle(),NULL));
#else
	return FALSE;
#endif
}
#endif	//	#ifdef REG_COPY_KEY

// Helper
//
#ifdef REG_COPY_KEY
BOOL CRegEntry::CopyKey(LPCTSTR szSrc, HKEY hDest, LPCTSTR szDestRoot, LPCTSTR szDestKey)
{
	if (IS_EMPTY(szSrc) || IS_EMPTY(szDestRoot) || IS_EMPTY(szDestKey))
	{
		return FALSE;
	}
	
	// Je cancatène les 2 chaines
	//
	TCHAR szDest[MAX_PATH+1];
	_tcscpy(szDest,szDestRoot);

	if (szDest[_tclen(szDest)-1] != _T('\\'))
	{
		_tcscat(szDest,_T("\\"));
	}

	_tcscat(szDest,szDestKey);

	// Et j'appelle l'autre méthode
	//
	return CopyKey(szSrc,hDest,szDest);
}
#endif	//	#ifdef REG_COPY_KEY

//--------------------------------------------------------------------------------
//--
//--	CRegValueList
//--
//--------------------------------------------------------------------------------

//	Libération
//
VOID CRegValueList::FreeList()
{
	deque<LPKEYVALUE>::iterator i;
	LPKEYVALUE pKey;
	
	for (i=_list.begin(); i!=_list.end();i++)
	{
		if (*i)
		{
			pKey = (*i);

			if (pKey->szValueName)
			{
				free(pKey->szValueName);
			}
			
			// Libération des chaînes de caractère
			//
			if (pKey->type == VALUE_STRING && pKey->Value.szValue)
			{
				free(pKey->Value.szValue);
			}

			// Des binaires
			else 
				// Libération des chaînes de caractère
				//
				if (pKey->type == VALUE_BINARY && pKey->Value.pValue)
				{
					free(pKey->Value.pValue);
				}
			
			free(pKey);
		}
	}
	
	_list.clear();	/* vraiment vide ... */
}

// Ajout
//

BOOL CRegValueList::AddValue(LPTSTR szName,LPKEYVALUE pValue)
{
	pValue->szValueName = _tcsdup(szName);
	
	size_t count = _list.size();
	_list.push_back(pValue);
	return (count <= _list.size());
}

// Une valeur entière
//
BOOL CRegValueList::AddValueInt(LPTSTR szName,int iValue, regValueType type)
{
	if (!szName)
	{
		return FALSE;
	}

	// Allocation de la structure
	//
	LPKEYVALUE pValue = (LPKEYVALUE)malloc(sizeof(KEYVALUE));

	// Pas assez de mémoire
	//
	if (!pValue)
	{
		return FALSE;
	}

	// Remplissage
	//
	memset(pValue,0,sizeof(KEYVALUE));
	pValue->type = type;
	pValue->Value.iValue = iValue;
	
	return AddValue(szName,pValue);
}

// Ajout d'une chaîne de caractères
//
BOOL CRegValueList::AddValueString(LPTSTR szName,LPCTSTR szValue)
{
	if (!szName || !szValue)
	{
		return FALSE;
	}

	/*
	// Est un booléen ?
	//
	if (0 == _tcscmp(szValue,STR_YES) ||
		0 == _tcscmp(szValue,STR_NO))
	{
		return AddValueBOOL(szName,!_tcscmp(szValue,STR_YES));
	}
	*/

	// Allocation de la structure
	//
	LPKEYVALUE pValue = (LPKEYVALUE)malloc(sizeof(KEYVALUE));
	
	// Pas assez de mémoire
	//
	if (!pValue)
	{
		return FALSE;
	}

	// Remplissage
	//
	memset(pValue,0,sizeof(KEYVALUE));
	pValue->type = VALUE_STRING;
	pValue->Value.szValue = _tcsdup(szValue);
	
	return AddValue(szName,pValue);
}

// Ajout d'un valeur "binaire"
//
BOOL CRegValueList::AddValueBinary(LPTSTR szName,LPVOID pValue, DWORD dwTypeValue, DWORD dwSize)
{
	if (!szName || !pValue || !dwSize)
	{
		return FALSE;
	}

	// Allocation de la structure
	//
	LPKEYVALUE pKeyValue = (LPKEYVALUE)malloc(sizeof(KEYVALUE));
	
	// Pas assez de mémoire
	//
	if (!pKeyValue)
	{
		return FALSE;
	}

	// Remplissage
	//
	memset(pKeyValue,0,sizeof(KEYVALUE));

	pKeyValue->type = VALUE_BINARY;
	pKeyValue->dwTypeValue = dwTypeValue;
	pKeyValue->dwValueSize = dwSize;
	
	// Copie du buffer
	//
	pKeyValue->Value.pValue = malloc(dwSize);
	memcpy(pKeyValue->Value.pValue,pValue,dwSize);
	
	return AddValue(szName,pKeyValue);
}



// Consultation
//
const CRegValueList::LPKEYVALUE CRegValueList::GetValue(size_t index)
{
	// Hors normes ...
	//
	if (index >= _list.size())
	{
		return NULL;
	}
	
	return (_list[index]);
}


//--------------------------------------------------------------------------------
//--
//--	CRegKeyList
//--
//--------------------------------------------------------------------------------

//	Libération
//
VOID CRegKeyList::FreeList()
{
	deque<LPTSTR>::iterator i;

	for (i=_list.begin(); i!=_list.end(); i++)
	{
		if (*i)
		{
			free(*i);
		}	
	}

	_list.clear();
}

// Ajout
//
BOOL CRegKeyList::AddKeyName(LPTSTR szNew)
{
	if (!szNew)
	{
		return FALSE;
	}

	LPTSTR szInter = _tcsdup(szNew);	

	if (!szInter)
	{
		return FALSE;
	}

	// Ajout
	//
	size_t count = _list.size();
	_list.push_back(szInter);	//	m_szList+=szInter;
	return (count <= _list.size());
}

// Consultation
//
size_t CRegKeyList::GetKeyName(DWORD dwIndex, LPTSTR szDest, size_t cbDest)
{
	// Hors normes ...
	//
	if (!szDest || !cbDest || dwIndex >= _list.size())
	{
		return 0;
	}
	
	// Assez de place ?
	//
	LPTSTR szInter = _list[dwIndex];
	size_t cbInter =  _tcslen(szInter);
	if (cbInter >= cbDest)
	{
		_tcsncpy_s(szDest, cbDest, szInter, cbDest);
		szDest[cbDest] = EOS;
		return cbDest;
	}
	
	// Tout copié !!!
	//
	_tcscpy_s(szDest, cbDest, szInter);
	return cbInter;
}

// EOF
