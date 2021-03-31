//---------------------------------------------------------------------------
//--	
//--	FICHIER	: titles.h
//--	
//--	AUTEUR	: J�r�me Henry-Barnaudi�re - JHB
//--	
//--	PROJET	: ldap2File
//--	
//---------------------------------------------------------------------------
//--	
//--	DESCRIPTIONS:
//--	
//--			D�finition de la classe titles
//--			Liste des titres/intitul�s de postes
//--	
//---------------------------------------------------------------------------
//--	
//--	MODIFICATIONS:
//--	-------------
//--
//--	09/07/2020 - JHB - Cr�ation
//--
//--	31/03/2021 - JHB - Version 21.3.8
//--
//---------------------------------------------------------------------------

#ifdef __LDAP_USE_ALLIER_TITLES_h__

#include "sharedConsts.h"

#ifndef __LDAP_2_FILE_TITLES_LIST_h__
#define __LDAP_2_FILE_TITLES_LIST_h__

namespace jhbLDAPTools {

	//
	// D�finition de la classe
	//
	class titles
	{
		// M�thodes publiques
	public:

		// Un intitul� de poste
		//
		class title
		{
		public:
			// Construction
			title()
				: id_{ "" }, label_{ "" }, manager_{ false }, description_{ "" }
			{}
			title(const string& id, const string& label, int responsable, const string& description) {
				id_ = id;
				label_ = label;
				manager_ = (1 == responsable);
				description_ = description;
			}

			// Destruction
			virtual ~title()
			{}

			// Acc�s
			//

			// Identifiant
			const char* id()
			{
				return id_.c_str();
			}

			// Libell�
			const char* label()
			{
				return label_.c_str();
			}

			// Manager ?
			bool isManager()
			{
				return manager_;
			}

			// Description
			const char* description()
			{
				return description_.c_str();
			}

		protected:
			// Donn�es membres
			//
			string			id_;			// ID (n'estpas forc�ment num�rique)
			string			label_;
			bool			manager_;		// Est-ce un manager ?
			string			description_;	// description
		};

		typedef title* LPAGENTTITLE;

		// Construction et destruction
		//
		titles(logFile* logs)
		{ logs_ = logs; }
		virtual ~titles()
		{ clear(); }

		// Vidage
		void clear();

		// Nombre d'elements
		size_t size()
		{ return titles_.size(); }

		// Ajout d'un "titre"
		bool add(const string& id, const string& label, int responsable, const string& description);

		// Recherche d'un service par son identifiant
		LPAGENTTITLE find(const char* id) {
			if (IS_EMPTY(id)) {
				return NULL;
			}
			string value(id);
			return find(value);
		}
		LPAGENTTITLE find(const string& id);

		// Methodes priv�es
	protected:


		// Donn�es membres priv�es
		//
	protected:

		logFile* logs_;

		// Liste
		deque<LPAGENTTITLE>		titles_;
	};
};	// jhbLDAPTools

#endif // __LDAP_2_FILE_TITLES_LIST_h__

#endif __LDAP_USE_ALLIER_TITLES_h__

// EOF