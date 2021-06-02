//---------------------------------------------------------------------------
//--
//--	FICHIER	: ODSFile.cpp
//--
//--	AUTEUR	: Jérôme Henry-Barnaudière - JHB
//--
//--	PROJET	: ldap2File
//--
//--    COMPATIBILITE : Win32 | Linux  Fedora (34 et +) / CentOS (7 & 8)
//--
//---------------------------------------------------------------------------
//--
//--	DESCRIPTION:
//--
//--			Implémentation de la classe ODSFile
//--			Génération d'un fichier au format Open Documents
//--
//---------------------------------------------------------------------------
//--
//--	MODIFICATIONS:
//--	-------------
//--
//--	17/12/2015 - JHB - Création
//--
//--	02/06/2021 - JHB - Version 21.6.8
//--
//---------------------------------------------------------------------------

#include "ODSFile.h"
#include "ODSConsts.h"

#include "sFileSystem.h"

//----------------------------------------------------------------------
//--
//-- Constantes privées
//--
//----------------------------------------------------------------------

// BUG éditeur Code::Blocks ...
#ifndef _WIN32
#ifndef __USE_CMD_LINE_ZIP__
#define __USE_CMD_LINE_ZIP__
#endif // __USE_CMD_LINE_ZIP__
#endif // _WIN32

//----------------------------------------------------------------------
//--
//-- Implémentation de la classe zipFile
//--
//----------------------------------------------------------------------

#ifdef __USE_CMD_LINE_ZIP__
bool ODSFile::zipFile::setTempFolder(const char* szFolder, string& msg)
{
	// Pas de dossier temp => on se positionne dans le dossier courant
	string zipFolder((IS_EMPTY(szFolder)) ? "." : szFolder);
	tempFolder_ = sFileSystem::merge(zipFolder, ZIP_TEMP_FOLDER);

	// Si le dosssier existe, on le supprime
	if (sFileSystem::is_directory(tempFolder_)) {
		if (false == sFileSystem::remove_all(tempFolder_)) {
			msg =  "Génération ODS - Impossible de vider l'ancien dossier temporaire '" + tempFolder_ + "'";
			return false;
		}
	}

	// Ok
	return true;
}
#endif // __USE_CMD_LINE_ZIP__

// Ouverture d'un fichier existant
//
bool ODSFile::zipFile::open(const char* fileName)
{
	if (IS_EMPTY(fileName)){
		return false;
	}

	// Déja un fichier ouvert ?
	if (file_ && srcPath_ != fileName) {
		close();
	}

	srcPath_ = "";

#ifdef __USE_CMD_LINE_ZIP__

	// Le fichier source doit exister
	if (false == sFileSystem::exists(fileName)){
		return false;
	}

	// Décompression du fichier source
	//

	// Génération de la ligne de commandes à partir des tokens
	string cmdLine(unzipAlias_->application());      // Application
	cmdLine+=" ";
	cmdLine+=unzipAlias_->command();                 // Ligne de commandes
	unzipAlias_->addToken(TOKEN_SRC_FILENAME, fileName, true);
	unzipAlias_->addToken(TOKEN_DEST_FOLDER, tempFolder_.c_str(), true);
    unzipAlias_->replace(cmdLine);                   // remplacment(s)

	// Execution de la ligne de commandes
	//std::system(cmd.c_str());
	std::system(cmdLine.c_str());

	// La decompression a t'elle eu lieu ?
	if (false == sFileSystem::is_directory(tempFolder_)) {
		return false;
	}
#else
	// On n'utilise plus l'objet ZipArchive qui ne fonctionne pas en modification
	// En remplacement, on utilise les api ZipFile::
	// auquel cas, file_ n'est plus un pointeur mais un booléen qui indique si le fichier est une archive zip valide ou non

	string sFile(fileName);
	//if (NULL == (file_ = ZipFile::Open(sFile))) {
	ZipArchive::Ptr zFile(NULL);
	try {
		zFile = ZipFile::Open(sFile);
	}
	catch (...) {
		// Une erreur ...
		zFile = NULL;
	}

	if (NULL == zFile){
		file_ = false;
		return false;
	}
#endif // __USE_CMD_LINE_ZIP__

	// Fichier "ouvert"
	file_ = true;
	srcPath_ = fileName;
	return true;
}

// Fermeture du fichier
//
void ODSFile::zipFile::close()
{
	if (file_) {
#ifdef __USE_CMD_LINE_ZIP__

		// Quelque chose à compresser ?
		if (false == sFileSystem::is_directory(tempFolder_)) {
			return;
		}

		// Suppression de l'ancienne source
		sFileSystem::remove(srcPath_);

		// Compression du dossier
		//

		// Génération de la ligne de commandes à partir des tokens
        string cmdLine("cd ");
        cmdLine += tempFolder_;                     // On se positionne dans le dossier
		cmdLine += ";";
        cmdLine += zipAlias_->application();        // Application
        cmdLine+=" ";
        cmdLine+=zipAlias_->command();              // Ligne de commandes
        zipAlias_->addToken(TOKEN_DEST_NAME, srcPath_.c_str(), true);
        zipAlias_->addToken(TOKEN_DEST_FOLDER, tempFolder_.c_str(), true);
        zipAlias_->replace(cmdLine);                // remplacment(s)

        // Execution
		std::system(cmdLine.c_str());
#endif  // #__USE_CMD_LINE_ZIP__
	}

#ifdef __USE_CMD_LINE_ZIP__
	    // Dans tous les cas, suppression du dossier
        sFileSystem::remove_all(tempFolder_);
#endif  // #__USE_CMD_LINE_ZIP__

	file_ = false;
	srcPath_ = "";
}

// Recherche dans l'archive d'un fichier par son nom
// On retourne l'index (ou -1 en cas d'erreur)
//
int ODSFile::zipFile::findFile(const char* fileName)
{
	if (!file_ || IS_EMPTY(fileName)) {
		// Non trouvé
		return -1;
	}

#ifdef __USE_CMD_LINE_ZIP__
	// Le fichier doit exister dans le dossier (pâs besoin d'aller dans les sous-dossiers)
	string path(_tempPath(fileName));
	return (sFileSystem::exists(path)?1:-1);
#else
	//ZipArchiveEntry::Ptr pentry = file_->GetEntry(fileName);

	// Retourne 1 si trouvé, -1 sinon (pas d'accès à l'index)
	//return ((pentry && !pentry->IsDirectory())?1:-1);
	string sFile(fileName);
	return (ZipFile::IsInArchive(srcPath_, sFile) ? 1 : -1);
#endif // #ifdef __USE_CMD_LINE_ZIP__
}

// Extraction d'un fichier particulier
//

// ... à partir de son nom
bool ODSFile::zipFile::extractFile(const char* fileName, const char* destFile)
{
	// Vérification des paramètres
	if (!file_ || IS_EMPTY(fileName) || IS_EMPTY(destFile)) {
		return false;
	}

	return extractFile(fileName, destFile);
}

// ... à partir de son nom
//
bool ODSFile::zipFile::extractFile(const string& srcName, const string& destFile)
{
	if ((0 == srcPath_.length() || 0 == srcName.length() || 0 == destFile.length())
		&& (-1 != findFile(srcName))) {
		// Paramètres invalides (ou fichier non encore ouvert)
		return false;
	}

#ifdef __USE_CMD_LINE_ZIP__
	// Le fichier existe, il suffit de le copier
	//
	std::string srcFile(_tempPath(srcName));
	sFileSystem::copy_file(srcFile, destFile);
#else
	try {
		ZipFile::ExtractFile(srcPath_, srcName, destFile);
	}
	catch (...) {
		// Une erreur lors de l'extraction
		return false;
	}
#endif // #ifdef __USE_CMD_LINE_ZIP__

	// Le fichier existe t'il ?
	if (sFileSystem::exists(destFile)) {
		// Le fichier doit être non vide
		return sFileSystem::file_size(destFile) > 0;
	}

	return false;
}

// Ajout d'un fichier à l'archive
//
bool ODSFile::zipFile::addFile(const char* srcFile, const char* destName)
{
	// Vérification des paramètres
	if (!file_ || IS_EMPTY(destName) || IS_EMPTY(srcFile)) {
		return false;
	}

	string sSrc(srcFile), sDst(destName);
	return addFile(sSrc, sDst);
}

bool ODSFile::zipFile::addFile(const string& srcFile, const string& destName)
{
	// Vérification des paramètres
	if (!file_) {
		return false;
	}

#ifdef __USE_CMD_LINE_ZIP__
	std::string destFile(_tempPath(destName));
	return sFileSystem::copy_file(srcFile, destFile);
#else
	//ZipArchiveEntry::Ptr newEntry(file_->CreateEntry(srcFile));
	try {
		ZipFile::AddFile(srcPath_, srcFile, destName);
	}
	catch (...) {
		// Une erreur ...
		return false;
	}

	// Ajouté ?
	return ZipFile::IsInArchive(srcPath_, destName);
#endif // #ifdef __USE_CMD_LINE_ZIP__
}

// Suppression de l'archive
//
bool ODSFile::zipFile::removeFile(const string& entryName)
{
	if (!file_ || 0 == srcPath_.length() || 0 == entryName.length()) {
		// Paramètres invalides (ou fichier non encore ouvert)
		return false;
	}

	// Retrait
	//
#ifdef __USE_CMD_LINE_ZIP__
	string fileName(_tempPath(entryName));
	return sFileSystem::remove(fileName);       // Suppression du fichier dans le dossier
#else
	try {
		ZipFile::RemoveEntry(srcPath_, entryName);
	}
	catch (...) {
		// Une erreur lors du retrait
		return false;
	}
#endif // #ifdef __USE_CMD_LINE_ZIP__


	// Existe t'il encore ?
	return (-1 == findFile(entryName.c_str()));
}

//----------------------------------------------------------------------
//--
//-- Implémentation de la classe ODSFile
//--
//----------------------------------------------------------------------

// Construction
//
ODSFile::ODSFile(const LPOPFI fileInfos, columnList* columns, confFile* parameters)
	:XMLFile(fileInfos, columns, parameters, true)
{
	// Initialisation des donneés membres
	//
	defaultContentFileName(contentFile_, false);
	lineIndex_ = 0;
	alternateRowCol_ = false;
	contentIndex_ = -1;
	tempFolder_ = "";

#ifdef __USE_CMD_LINE_ZIP__
	// zipAlias_ = unzipAlias_ = NULL;
#endif // __USE_CMD_LINE_ZIP__

}

// Destruction
//
ODSFile::~ODSFile()
{}

// Création / initialisation(s)
//
bool ODSFile::create()
{
	// Intialisations et ouverture du fichier
	//
	if (!XMLFile::init()){
		return false;
	}

	// La couleur des lignes doit-elle etre alternee ?
	alternateRowCol_ = (templateFile_.npos != templateFile_.find(XML_TEMPLATE_FILE_ALTERNATE));

	// OK
	return true;
}

// Noms des fichiers
//
void ODSFile::defaultContentFileName(string& out, bool shortName)
{
	if (shortName){
		out = ODS_CONTENT_FILENAME;
		return;
	}

	// Récupération du dossier "temp"
	out = sFileSystem::merge(folders_->find(folders::FOLDER_TYPE::FOLDER_TEMP)->path(), ODS_CONTENT_FILENAME);
}

void ODSFile::templateFileName(string& out, const char* name, bool shortName)
{
	if (shortName){
		out = (IS_EMPTY(name)? ODS_TEMPLATE_FILENAME :name);
		return;
	}

	// Chemin complet vers le fichier
	char sp(FILENAME_SEP);
	string dest(folders_->find(folders::FOLDER_TYPE::FOLDER_TEMPLATES)->path());
	/*
	dest += sp;
	dest += STR_FOLDER_TEMPLATES;
	*/
	dest += sp;
	dest += (IS_EMPTY(name)? ODS_TEMPLATE_FILENAME :name);
	out = dest;
}

// Enregistrement de la ligne courante
//
bool ODSFile::saveLine(bool header, LPAGENTINFOS agent)
{
	if (!strlen(sheetRoot_.name())){
		return false;
	}

	// Création de la ligne
	//
	pugi::xml_node row, cell, val;
	row = sheetRoot_.append_child(ODS_SHEET_ROW_NODE);
	row.append_attribute(ODS_ROW_STYLE_ATTR) = (header? ROW_STYLE_HEADER_VAL: ROW_STYLE_DEFAULT_VAL);

	// Couleur & style de la ligne
	string cellStyleName(CELL_TYPE_LINE);
	if (header){
		cellStyleName = CELL_TYPE_HEADER;
	}
	else{

		if (columns_->orgModeOn()){
			cellStyleName = CELL_TYPE_LINE;
		}
		else{
			if (alternateRowCol_ && (lineIndex_ % 2)){
				cellStyleName = CELL_TYPE_ALTERNATE_LINE;
			}
		}
	}

	// Ajout de toutes les valeurs visibles
	//
	columnList::LPCOLINFOS col(NULL);
	LPXMLCELL pCell(NULL);
	string fullLink(""), sValue("");
	size_t colMax = columns_->size();
	IMGSERVER photoServer;
	configurationFile_->imagesServer(photoServer);

	for (size_t colIndex(0); colIndex < colMax; colIndex++){
		col = columns_->at(colIndex);

		// Seules les colonnes visibles figureront dans le fichier de sortie
		if (col->visible()){
			cell = row.append_child(ODS_SHEET_CELL_NODE);

			// la première valeur..
			pCell = &(line_[colIndex]);

			// style de la cellule
			//cell.append_attribute(ODS_COL_STYLE_ATTR) = ((!header && col->hyperLink())? CELL_TYPE_DEFAULT :cellStyleName.c_str());
			cell.append_attribute(ODS_COL_STYLE_ATTR) = cellStyleName.c_str();

			// Le valeur de type numerique ne sont enregistrées comme telles
			// qu'à la condition qu'elles ne soient pas multivaluées !!!
			if (!header && col->numeric() && !pCell->_next && pCell->_value.size()){
				cell.append_attribute(ODS_CELL_TYPE_ATTR) = CELL_TYPE_FLOAT_VAL;
				cell.append_attribute(ODS_CELL_VAL_ATTR) = pCell->_value.c_str();
			}
			else{
				cell.append_attribute(ODS_CELL_TYPE_ATTR) = CELL_TYPE_STRING_VAL;
			}

			// Gestion de toutes les valeurs
			while (pCell){
				if (pCell->_value.size()){
					val = cell.append_child(ODS_CELL_TEXT_NODE);

					// J'ai une valeur
					if (!header && col->hyperLink()){
						// Un lien hyper texte
						val = val.append_child(ODS_CELL_TEXT_LINK_NODE);

						// lien vers ...
						if (col->imageLink()){
							// Une image
							fullLink = photoServer.URL(photoServer.shortFileName(pCell->_value.c_str()));
						}
						else{
							fullLink = (col->emailLink() ? "mailto:" : "http://");
							fullLink += pCell->_value;
						}
						val.append_attribute(ODS_CELL_LINK_ATTR) = fullLink.c_str();
					}

					// valeur simple / ou valeur affichée sur le lien
					val.text().set(pCell->_value.c_str());
				}
				else{
					// Il n'y a pas de valeurs
					// on regarde si les valeurs suivantes sont aussi vides
					size_t nextValid(1+colIndex);
					while (nextValid < colMax && 0 ==line_[nextValid]._value.size()){
						nextValid++;
					}

					// J'en ai plusieurs
					if (nextValid > (1 + colIndex)){
                        sValue = charUtils::itoa(nextValid - colIndex);
						cell.append_attribute(ODS_CELL_REPEATED_ATTR) = sValue.c_str();
					}

					// Ai je atteint la fin du tableau ?
					colIndex = (nextValid >= columns_->size() ? nextValid : nextValid - 1);
				}

				// Une autre valeur ?
				pCell = pCell->_next;
			}
		}
	}

	outputFile::_saveLine(header);

	// On repart a "0"
	_emptyLine();
	colIndex_ = 0;
	lineIndex_ += (header ? 0 : 1);
	return true;
}

// Création d'une ligne d'entete
//
void ODSFile::_addHeader()
{
	// La ligne est vierge
	_emptyLine();

	// Creation des colonnes
	//
	int visibleIndex(0);
	columnList::LPCOLINFOS col(NULL);
	for (size_t colIndex(0); colIndex < columns_->size(); colIndex++){
		col = columns_->at(colIndex);

		add(col->name_);
		visibleIndex++;
	}

	// Enregistrement de la ligne ...
	saveLine(true);
}

// Initialisation du fichier à générer
//
bool ODSFile::_initContentFile()
{

	//
	// Algo 3 - final (ouf !!!!)
	//
	// Cette fois nous allons agir en 3 étapes :
	//	1 - Copie du fichier de référence dans le fichier destination (on travaille avec les fichiers ODS)
	//		& extraction du fichier de contenu "vierge"
	//		::_initContentFile
	//
	//  2 - Génération du fichier de contenu à partir de la source "vierge"
	//		::_openContentFile et autre méthodes
	//
	//	3 - Retrait du fichier de contenu (du fichier destination)
	//		::_endOfContentFile
	//
	//  4 - Insertion du "nouveau" fichier de contenu
	//		::_endContentFile
	//
	//	5 - Enregistrement et fermeture du zip
	//		::_endOfContentFile
	//
	//	Plus simple et plus portable ...
	//

#ifndef _GEN_DOC_

#ifdef __USE_CMD_LINE_ZIP__
	// Le dossier zip temporaire est un sous-dossier du dossier temp
	//
	string msg("");
	if (false == destZip_.setTempFolder(folders_->find(folders::FOLDER_TYPE::FOLDER_TEMP)->path(), msg)) {
		if (logs_) {
			logs_->add(logs::TRACE_TYPE::ERR, msg.c_str());
		}
		return false;
	}
#endif // __USE_CMD_LINE_ZIP__


	// 1 - Copie du fichier de référence
	string newName(fileName());
	if (false == sFileSystem::copy_file(templateFile_, newName)) {
		if (logs_) {
			logs_->add(logs::TRACE_TYPE::ERR, "Pas de génération du fichier ODS : Impossible de copier le fichier %s", templateFile_.c_str());
		}

		return false;
	}

	// Ouverture du zip
	if (false == (destZip_.open(newName))) {
		if (logs_) {
			logs_->add(logs::TRACE_TYPE::ERR, "Impossible d'ouvrir la copie du fichier modele %s", newName.c_str());
		}

		// Suppression du fichier
		sFileSystem::remove(newName);

		return false;
	}

	// 2 - Extraction du fichier "contenu" modèle
	//

	// son "index"
	string shortName;
	defaultContentFileName(shortName);

	if (-1 == destZip_.findFile(shortName)) {
		if (logs_) {
			logs_->add(logs::TRACE_TYPE::ERR, "Impossible de trouver le fichier '%s' dans le modèle '%s'", shortName.c_str(), templateFile_.c_str());
		}

		return false;
	}

	if (false == destZip_.extractFile(shortName, contentFile_)) {
		if (logs_) {
			logs_->add(logs::TRACE_TYPE::ERR, "Impossible d'extraire le fichier '%s' dans le modèle '%s'", shortName.c_str(), templateFile_.c_str());
		}

		return false;
	}

	// Extrait avec succès
#endif // _GEN_DOC_
	return true;
}

// Ouverture du fichier à générer
//
bool ODSFile::_openContentFile()
{
#ifdef _GEN_DOC_
	// Création du document
	//

	TCHAR szValue[1024];

	// Déclaration "personnelles"
	pugi::xml_node decl = XMLContentFile_.prepend_child(pugi::node_declaration);
	decl.append_attribute("version") = "1.0";
	decl.append_attribute("encoding") = "UTF-8";

	// Base du document
	docRoot_ = XMLContentFile_.append_child(ODSfile__ROOT_NODE);

	// les styles
	stylesRoot_ = docRoot_.append_child(ODSfile__STYLE_NODE);

	// Pointeur sur le contenu
	sheetsRoot_ = XMLContentFile_.append_child(ODS_BODY_NODE);
	sheetsRoot_ = sheetsRoot_.append_child(ODS_SPREADSHEET_NODE);
#else
	// Recherche des pointeurs dans le fichier
	//
	pugi::xml_parse_status result;
	pugi::xml_node node;
	if (pugi::status_ok != (result = XMLContentFile_.load_file(contentFile_.c_str()).status)){
		// Impossible d'ouvrir le fichier
		return false;
	}

	// Quelques attributs à ajouter dans l'entete
	pugi::xml_node decl = XMLContentFile_.prepend_child(pugi::node_declaration);
	decl.append_attribute("version") = "1.0";
	//decl.append_attribute("encoding") = "Windows-1250";
	decl.append_attribute("encoding") = "UTF-8";

	docRoot_ = XMLContentFile_.child(ODS_FILE_ROOT_NODE);
	if (IS_EMPTY(docRoot_.name())){
		if (logs_){
			logs_->add(logs::TRACE_TYPE::ERR, "Le modèle '%s' n'est pas au bon format", templateFile_.c_str());
		}
		return false;
	}

	// Les styles
	stylesRoot_ = docRoot_.child(ODS_FILE_STYLE_NODE);

	// Mes onglets
	sheetsRoot_ = docRoot_.child(ODS_BODY_NODE);
	if (IS_EMPTY(sheetsRoot_.name())){
		if (logs_){
			logs_->add(logs::TRACE_TYPE::ERR, "Le modèle '%s' n'est pas au bon format", templateFile_.c_str());
		}
		return false;
	}
	sheetsRoot_ = sheetsRoot_.child(ODS_SPREADSHEET_NODE);
#endif // #ifdef _GEN_DOC_

	// Ajout de styles des colonnes
	//
	columnList::LPCOLINFOS col(NULL);
	char value[20];
	pugi::xml_node style;
	for (size_t colIndex(0); colIndex < columns_->size(); colIndex++){
		col = columns_->at(colIndex);

		// Seules colonnes visibles figureront dans le fichier de sortie
		if (col->visible()){
			style = stylesRoot_.append_child(ODS_STYLE_NODE);

			// Nom de la colonne
#ifdef _WIN32
			sprintf_s(value, 19, STYLE_NAME_COL_VAL, colIndex + 1);
#else
            sprintf(value, STYLE_NAME_COL_VAL, (int)(colIndex + 1));
#endif // _WIN32
			style.append_attribute(ODS_STYLE_NAME_ATTR) = value;

			// c'est une colonne ...
			style.append_attribute(ODS_STYLE_FAMILY_ATTR) = STYLE_FAMILY_COL;

			// on descend ...
			style = style.append_child(ODS_STYLE_COLUMN_PROP_NODE);
			style.append_attribute(ODS_COLUMN_PROP_BREAK_ATTR) = COLUMN_PROP_BREAK_VAL;

			// la largeur en cm.
			if (col->width_ != COL_DEF_WITDH){
				//sprintf_s(value, 19, "%ldcm", )
				stringstream str;
				str << col->width_ << "cm";
				string value;
				str >> value;
				style.append_attribute(ODS_COLUMN_PROP_WIDTH_ATTR) = value.c_str();
			}
		}
	}

	// Retrait des onglets existants
	sheetRoot_ = sheetsRoot_.child(ODS_SHEET_NODE);
	while (!IS_EMPTY(sheetRoot_.name())){
		// suppression
		sheetsRoot_.remove_child(sheetRoot_);
		//sheetRoot_ = sheetRoot_.next_sibling();
		sheetRoot_ = sheetsRoot_.child(ODS_SHEET_NODE);
	}

	_createSheet(/*tabName*/);

	// Ok
	return true;
}

// Fin des traitements
//
bool ODSFile::_endContentFile()
{
	bool created(false);	// Pas enocre crée ...

	// Nom du fichier de contenu dans la source ods
	string shortName;
	defaultContentFileName(shortName);

	// 3 - Retrait du fichier de contenu
	if (true == (destZip_.removeFile(shortName))) {
		// 3 - Insertion du "nouveau" contenu
		if (false == (destZip_.addFile(contentFile_, shortName))) {
			if (logs_) {
				logs_->add(logs::TRACE_TYPE::ERR, "Impossible d'ajouter le fichier de contenu '%s' au fichier ods destination", shortName.c_str());
			}
		}
		else {
			// Ouf!!!
			created = true;
		}
	}
	else {
		if (logs_) {
			logs_->add(logs::TRACE_TYPE::ERR, "Impossible de supprimer le contenu du fichier modèle");
		}
	}

	// Je n'ai plus besoin du fichier de contenu
#ifndef _DEBUG
	sFileSystem::remove(contentFile_);
#endif // _DEBUG

	// 5 - fermeture (et compression si besoin)
	destZip_.close();

	// Terminé avec succés (ou pas)
	return created;
}

// Un nouvel onglet ...
//
bool ODSFile::addSheet(string& sheetName, bool withHeader, bool firstSheet)
{
	// Le docucment est-il en cours de traitement ?
	if (!strlen(sheetRoot_.name())){
		return false;
	}

	// Nom de l'onglet
	//
	string validName("");
	if (-1 != fileInfos_->sheetNameLen_ && sheetName.size() > (size_t)fileInfos_->sheetNameLen_) {
		validName = charUtils::shorten(sheetName, (size_t)fileInfos_->sheetNameLen_);
	}
	else{
		validName = sheetName;
	}

	// Un caractère à remplacer ?
	size_t pos(0);
	while (validName.npos != (pos = validName.find("/"))) {
		validName.replace(pos, 1, "-");
	}

#ifdef _WIN32
	encoder_.convert_toUTF8(validName, false);
#endif // _WIN32

	return _createSheet(validName.c_str(), withHeader);
}

// Création d'un onglet
//
bool ODSFile::_createSheet(const char* name, bool withHeader, bool sizeColumns)
{
	// On repart en haut de l'onglet
	lineIndex_ = 0;

	// Création du noeud
	//
	sheetRoot_ = sheetsRoot_.append_child(ODS_SHEET_NODE);
	sheetRoot_.append_attribute(ODS_SHEET_STYLE_ATTR) = ODS_SHEET_STYLE_TA1_VAL;
	sheetRoot_.append_attribute(ODS_SHEET_PRINT_ATTR) = ODS_VAL_NO;

	// son nom
	_setSheetName(name);

	// Définition des colonnes
	//
	if (sizeColumns)
	{
		columnList::LPCOLINFOS col(NULL);
		char value[20];
		pugi::xml_node node;
		for (size_t colIndex(0); colIndex < columns_->size(); colIndex++){
			col = columns_->at(colIndex);

			// Seules les colonnes visibles figureront dans le fichier de sortie
			if (col->visible()){
				node = sheetRoot_.append_child(ODS_SHEET_COL_NODE);

				// Nom de la colonne
#ifdef _WIN32
				sprintf_s(value, 19, COL_STYLE_BASE_VAL, colIndex + 1);
#else
				sprintf(value, COL_STYLE_BASE_VAL, (int)(colIndex + 1));
#endif // _WIN32
				node.append_attribute(ODS_COL_STYLE_ATTR) = value;

				node.append_attribute(ODS_COL_CELL_ATTR) = CELL_TYPE_HEADER;
			}
		}
	}

	// Ajout de l'entete
	if (withHeader){
		_addHeader();
	}

	// Crée avec succès
	return true;
}

// Nom de l'onglet
//
void ODSFile::_setSheetName(const char* sheetName)
{
	// L'onglet existe ?
	if (strlen(sheetRoot_.name()) && !IS_EMPTY(sheetName)){
		/*string validName(sheetName);
		encoder_.toUTF8(validName, false);
		sheetRoot_.append_attribute(ODS_SHEET_NAME_ATTR) = validName.c_str();*/
		sheetRoot_.append_attribute(ODS_SHEET_NAME_ATTR) = sheetName;
	}
}

// EOF
