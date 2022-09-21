//---------------------------------------------------------------------------
//--
//--	FICHIER	: titles.h
//--
//--	AUTEUR	: Jérôme Henry-Barnaudière - JHB
//--
//--	PROJET	: ldap2File
//--
//--    COMPATIBILITE : Win32 | Linux  Fedora (34 et +) / CentOS (7 & 8)
//--
//---------------------------------------------------------------------------
//--
//--	DESCRIPTIONS:
//--
//--			Définition de la classe titles
//--			Liste des titres/intitulés de postes
//--
//---------------------------------------------------------------------------
//--
//--	MODIFICATIONS:
//--	-------------
//--
//--	09/07/2020 - JHB - Création
//--
//--	21/09/2022 - JHB - Version 22.6.5
//--
//---------------------------------------------------------------------------

#ifdef __LDAP_USE_ALLIER_TITLES__

#include "sharedConsts.h"

#ifndef __LDAP_2_FILE_TITLES_LIST_h__
#define __LDAP_2_FILE_TITLES_LIST_h__   1

namespace jhbLDAPTools {

	//
	// Définition de la classe
	//
	class titles
	{
		// Méthodes publiques
	public:

		// Un intitulé de poste
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

			// Accès
			//

			// Identifiant
			const char* id()
			{
				return id_.c_str();
			}

			// Libellé
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
			// Données membres
			//
			string			id_;			// ID (n'estpas forcément numérique)
			string			label_;
			bool			manager_;		// Est-ce un manager ?
			string			description_;	// description
		};

		typedef title* LPAGENTTITLE;

		// Construction et destruction
		//
		titles(logs* pLogs)
		{ logs_ = pLogs; }
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
				return nullptr;
			}
			string value(id);
			return find(value);
		}
		LPAGENTTITLE find(const string& id);

		// Methodes privées
	protected:


		// Données membres privées
		//
	protected:

		logs*					logs_;

		// Liste
		deque<LPAGENTTITLE>		titles_;
	};
};	// jhbLDAPTools

#endif // __LDAP_2_FILE_TITLES_LIST_h__

#endif // __LDAP_USE_ALLIER_TITLES__

// EOF
