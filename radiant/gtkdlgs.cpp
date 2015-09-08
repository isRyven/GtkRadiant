/*
   Copyright (c) 2001, Loki software, inc.
   All rights reserved.

   Redistribution and use in source and binary forms, with or without modification,
   are permitted provided that the following conditions are met:

   Redistributions of source code must retain the above copyright notice, this list
   of conditions and the following disclaimer.

   Redistributions in binary form must reproduce the above copyright notice, this
   list of conditions and the following disclaimer in the documentation and/or
   other materials provided with the distribution.

   Neither the name of Loki software nor the names of its contributors may be used
   to endorse or promote products derived from this software without specific prior
   written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
   DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE FOR ANY
   DIRECT,INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
   ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

//
// Some small dialogs that don't need much
//
// Leonardo Zide (leo@lokigames.com)
//

#include "stdafx.h"
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include <glib/gi18n.h>

#ifdef _WIN32
#include <gdk/gdkwin32.h>
#endif

#ifdef _WIN32
#include <shellapi.h>
#endif

// =============================================================================
// Color selection dialog

qboolean DoColor( int iIndex ){
	static bool bColorOpen = false;

	if ( bColorOpen ) {
		Sys_FPrintf( SYS_WRN, "DoColor dialog is already open\n" );
		return false;
	}

	bColorOpen = true;

	if ( color_dialog( g_pParentWnd->m_pWidget, g_qeglobals.d_savedinfo.colors[iIndex] ) ) {
		/*
		** scale colors so that at least one component is at 1.0F
		** if this is meant to select an entity color
		*/
		if ( iIndex == COLOR_ENTITY ) {
			float largest = 0.0F;

			if ( g_qeglobals.d_savedinfo.colors[iIndex][0] > largest ) {
				largest = g_qeglobals.d_savedinfo.colors[iIndex][0];
			}
			if ( g_qeglobals.d_savedinfo.colors[iIndex][1] > largest ) {
				largest = g_qeglobals.d_savedinfo.colors[iIndex][1];
			}
			if ( g_qeglobals.d_savedinfo.colors[iIndex][2] > largest ) {
				largest = g_qeglobals.d_savedinfo.colors[iIndex][2];
			}

			if ( largest == 0.0F ) {
				g_qeglobals.d_savedinfo.colors[iIndex][0] = 1.0F;
				g_qeglobals.d_savedinfo.colors[iIndex][1] = 1.0F;
				g_qeglobals.d_savedinfo.colors[iIndex][2] = 1.0F;
			}
			else
			{
				float scaler = 1.0F / largest;

				g_qeglobals.d_savedinfo.colors[iIndex][0] *= scaler;
				g_qeglobals.d_savedinfo.colors[iIndex][1] *= scaler;
				g_qeglobals.d_savedinfo.colors[iIndex][2] *= scaler;
			}
		}

		Sys_UpdateWindows( W_ALL );
		bColorOpen = false;
		return true;
	}
	else {
		bColorOpen = false;
		return false;
	}
}

// =============================================================================
// Project settings dialog

static void UpdateBSPCommandList( GtkWidget *dialog );

static void DoProjectAddEdit( bool edit, GtkWidget *parent ){
	GtkWidget *dialog, *vbox, *label, *table;
	GtkWidget *cmd, *text, *content_area;
	gint response_id;
	GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;

	if ( edit ) {
		dialog = gtk_dialog_new_with_buttons( _( "Edit Command" ), NULL, flags, NULL );
	}
	else{
		dialog = gtk_dialog_new_with_buttons( _( "Add Command" ), NULL, flags, NULL );
	}
	gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "OK" ), GTK_RESPONSE_OK );
	gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "Cancel" ), GTK_RESPONSE_CANCEL );

	load_window_pos( dialog, g_PrefsDlg.mWindowInfo.posMapInfoWnd );

	content_area = gtk_dialog_get_content_area( GTK_DIALOG( dialog ) );

	vbox = gtk_box_new( GTK_ORIENTATION_VERTICAL, 5 );
	gtk_container_add( GTK_CONTAINER( content_area ), vbox );
	gtk_container_set_border_width( GTK_CONTAINER( vbox ), 5 );
	gtk_widget_show( vbox );

	table = gtk_grid_new();
	gtk_box_pack_start( GTK_BOX( vbox ), table, TRUE, TRUE, 0 );
	gtk_grid_set_row_spacing( GTK_GRID( table ), 5 );
	gtk_grid_set_column_spacing( GTK_GRID( table ), 5 );
	gtk_widget_show( table );

	label = gtk_label_new( _( "Menu text" ) );
	gtk_grid_attach( GTK_GRID( table ), label, 0, 0, 1, 1 );
	gtk_widget_set_halign( label, GTK_ALIGN_START );
	gtk_widget_show( label );

	label = gtk_label_new( _( "Command" ) );
	gtk_grid_attach( GTK_GRID( table ), label, 0, 1, 1, 1 );
	gtk_widget_set_halign( label, GTK_ALIGN_START );
	gtk_widget_show( label );

	text = gtk_entry_new();
	gtk_grid_attach( GTK_GRID( table ), text, 1, 0, 1, 1 );
	gtk_widget_set_hexpand( text, TRUE );
	gtk_widget_show( text );
	g_object_set_data( G_OBJECT( dialog ), "text", text );

	cmd = gtk_entry_new();
	gtk_grid_attach( GTK_GRID( table ), cmd, 1, 1, 1, 1 );
	gtk_widget_set_hexpand( cmd, TRUE );
	gtk_widget_show( cmd );
	g_object_set_data( G_OBJECT( dialog ), "cmd", cmd );

	if ( edit ) {
		GtkTreeView* view = GTK_TREE_VIEW( g_object_get_data( G_OBJECT( parent ), "view" ) );
		GtkTreeSelection* selection = gtk_tree_view_get_selection( view );
		GtkTreeIter iter;
		GtkTreeModel* model;
		if ( gtk_tree_selection_get_selected( selection, &model, &iter ) ) {
			char* key;
			gtk_tree_model_get( model, &iter, 0, &key, -1 );
			const char* value = ValueForKey( g_qeglobals.d_project_entity, key );
			gtk_entry_set_text( GTK_ENTRY( text ), key );
			gtk_entry_set_text( GTK_ENTRY( cmd ), value );
			g_free( key );
		}
	}


	gtk_widget_show( dialog );

	response_id = gtk_dialog_run( GTK_DIALOG( dialog ) );

	if ( response_id == GTK_RESPONSE_OK ) 
	{
		const char* key = gtk_entry_get_text( GTK_ENTRY( text ) );
		const char* value = gtk_entry_get_text( GTK_ENTRY( cmd ) );

		if ( strlen( key ) <= 0 || strlen( value ) <= 0 ) {
			Sys_Printf( "Command not added\n" );
		}
		else
		{
			if ( edit ) {
				SetKeyValue( g_qeglobals.d_project_entity, key, value );
				FillBSPMenu();
			}
			else
			{
				if ( key[0] == 'b' && key[1] == 's' && key[2] == 'p' ) {
					SetKeyValue( g_qeglobals.d_project_entity, key, value );
					FillBSPMenu();
				}
				else{
					Sys_Printf( "BSP commands must be preceded by \"bsp\"" );
				}
			}

			UpdateBSPCommandList( parent );
		}
	}

	gtk_widget_destroy( dialog );
}

static void UpdateBSPCommandList( GtkWidget *dialog ){
	GtkListStore* store = GTK_LIST_STORE( g_object_get_data( G_OBJECT( dialog ), "bsp_commands" ) );

	gtk_list_store_clear( store );

	for ( epair_t* ep = g_qeglobals.d_project_entity->epairs; ep != NULL; ep = ep->next )
	{
		if ( ep->key[0] == 'b' && ep->key[1] == 's' && ep->key[2] == 'p' ) {
			GtkTreeIter iter;
			gtk_list_store_append( store, &iter );
			gtk_list_store_set( store, &iter, 0, ep->key, -1 );
		}
	}
}

static void project_add( GtkWidget *widget, gpointer data ){
	GtkWidget *dlg = GTK_WIDGET( data );
	DoProjectAddEdit( false, dlg );
	UpdateBSPCommandList( dlg );
}

static void project_change( GtkWidget *widget, gpointer data ){
	GtkWidget *dlg = GTK_WIDGET( data );
	DoProjectAddEdit( true, dlg );
	UpdateBSPCommandList( dlg );
}

static void project_remove( GtkWidget *widget, gpointer data ){
	GtkWidget* project = GTK_WIDGET( data );

	GtkTreeView* view = GTK_TREE_VIEW( g_object_get_data( G_OBJECT( project ), "view" ) );
	GtkTreeSelection* selection = gtk_tree_view_get_selection( view );
	GtkTreeIter iter;
	GtkTreeModel* model;
	if ( gtk_tree_selection_get_selected( selection, &model, &iter ) ) {
		char* key;
		gtk_tree_model_get( model, &iter, 0, &key, -1 );
		DeleteKey( g_qeglobals.d_project_entity, key );
		g_free( key );

		char* index = gtk_tree_model_get_string_from_iter( model, &iter );
		Sys_Printf( "Selected %s\n", index );
		g_free( index );

		UpdateBSPCommandList( project );
		FillBSPMenu();
	}
}

static const char* sQ3ComboItem = "Quake III Arena";
static const char* sTAComboItem = "Quake III: Team Arena";
static const char* sModComboItem = "Custom Quake III modification";
static const char* sWolfComboItem = "Return To Castle Wolfenstein";
static const char* sWolfModComboItem = "Custom RTCW modification";
static const char* sHLComboItem = "Half-life";
static const char* sHLModComboItem = "Custom Half-life modification";

static const char* sWolfSPCombo = "Single Player mapping mode";
static const char* sWolfMPCombo = "Multiplayer mapping mode";

// Arnout
// HARD-CODED ET HACK
static const char* sETComboItem = "Wolfenstein: Enemy Territory";
static const char* sETModComboItem = "Custom ET modification";

// RIANT
// HARD-CODED JK2 HACK
static const char* sJK2ComboItem = "Jedi Knight II Outcast";
static const char* sJK2ModComboItem = "Custom JK2 modification";
static const char* sJK2SPCombo = "Single Player mapping mode";
static const char* sJK2MPCombo = "Multiplayer mapping mode";

// TTimo
// HARD-CODED JA HACK
static const char* sJAComboItem = "Jedi Knight Jedi Academy";
static const char* sJAModComboItem = "Custom JA modification";
static const char* sJASPCombo = "Single Player mapping mode";
static const char* sJAMPCombo = "Multiplayer mapping mode";

// RIANT
// HARD-CODED STVEF2 HACK
static const char* sSTVEFComboItem = "Star Trek Voyager : Elite Force";
static const char* sSTVEFModComboItem = "Custom Elite Force modification";
static const char* sSTVEFSPCombo = "Single Player mapping mode";
static const char* sSTVEFMPCombo = "Holo Match mapping mode";

// RIANT
// HARD-CODED SOF2 HACK
static const char* sSOF2ComboItem = "Soldier of Fortune II - Double Helix";
static const char* sSOF2ModComboItem = "Custom Sof2 modification";
static const char* sSOF2SPCombo = "Single Player mapping mode";
static const char* sSOF2MPCombo = "Multiplayer mapping mode";

struct gamemode_s {
	const char *gameFile;
	const char *name;
	const char *mode;
};
typedef struct gamemode_s gamemode_t;

gamemode_t gameModeList[] = {
	{ "wolf.game", sWolfSPCombo, "sp" },
	{ "wolf.game", sWolfMPCombo, "mp" },

	{ "jk2.game", sJK2SPCombo, "sp" },
	{ "jk2.game", sJK2MPCombo, "mp" },

	{ "ja.game", sJASPCombo, "sp" },
	{ "ja.game", sJAMPCombo, "mp" },

	{ "stvef.game", sSTVEFSPCombo, "sp" },
	{ "stvef.game", sSTVEFMPCombo, "mp" },

	{ "sof2.game", sSOF2SPCombo, "sp" },
	{ "sof2.game", sSOF2MPCombo, "mp" },

};

struct game_s {
	const char *gameFile;
	const char *name;
	const char *fs_game;
	qboolean base;
	qboolean custom; //ie Custom Quake III modification
};
typedef struct game_s game_t;

game_t gameList[] = {
	{ "q3.game", sQ3ComboItem, "baseq3", qtrue, qfalse },
	{ "q3.game", sTAComboItem, "missionpack", qfalse, qfalse },
	{ "q3.game", "Defrag", "defrag", qfalse, qfalse },
	{ "q3.game", sModComboItem, "", qfalse, qtrue },

	{ "wolf.game", sWolfComboItem, "main", qtrue, qfalse },
	{ "wolf.game", sWolfModComboItem, "", qfalse, qfalse },

	{ "hl.game", sHLComboItem, "valve", qtrue, qfalse },
	{ "hl.game", sHLModComboItem, "", qfalse, qtrue },
	
	{ "et.game", sETComboItem, "etmain", qtrue, qfalse },
	{ "et.game", sETModComboItem, "", qfalse, qtrue },

	{ "jk2.game", sJK2ComboItem, "base", qtrue, qfalse },
	{ "jk2.game", sJK2ModComboItem, "", qfalse, qtrue },

	{ "ja.game", sJAComboItem, "base", qtrue, qfalse },
	{ "ja.game", sJAModComboItem, "", qfalse, qtrue },

	{ "stvef.game", sSTVEFComboItem, "baseEf", qtrue, qfalse },
	{ "stvef.game", sSTVEFModComboItem, "", qfalse, qtrue },

	{ "sof2.game", sSOF2ComboItem, "base", qtrue, qfalse },
	{ "sof2.game", sSOF2ModComboItem, "", qfalse, qtrue },

};

GList *newMappingModesListForGameFile( Str & mGameFile ){
	GList *mode_list;
	int x;

	mode_list = NULL;
	for( x = 0; x < G_N_ELEMENTS( gameModeList ); x++ )
	{
		if( strcmp( mGameFile.GetBuffer(), gameModeList[x].gameFile ) == 0 ){
			mode_list = g_list_append( mode_list, &gameModeList[x] );
		}
	}
	return mode_list;
}

GList *newModListForGameFile( Str & mGameFile ){
	GList *mod_list;
	int x;

	mod_list = NULL;
	for( x = 0; x < G_N_ELEMENTS( gameList ); x++ )
	{
		if( strcmp( mGameFile.GetBuffer(), gameList[x].gameFile ) == 0 ){
			mod_list = g_list_append( mod_list, &gameList[x] );
		}
	}
	return mod_list;
}

void OnSelchangeComboWhatgame( GtkWidget *widget, gpointer data ){
	GtkWidget *fs_game_entry;
	GtkWidget *fs_game_label;
	GtkWidget *game_select;
	int x;
	const gchar *fs_game;

	game_select = GTK_WIDGET( g_object_get_data( G_OBJECT( data ), "game_select" ) );
	fs_game = gtk_combo_box_get_active_id( GTK_COMBO_BOX( GTK_COMBO_BOX_TEXT( game_select ) ) );

	if( !fs_game ){

		return;
	}
	fs_game_entry = GTK_WIDGET( g_object_get_data( G_OBJECT( data ), "fs_game_entry" ) );
	fs_game_label = GTK_WIDGET( g_object_get_data( G_OBJECT( data ), "fs_game_label" ) );

	for( x = 0; x < G_N_ELEMENTS( gameList ); x++ )
	{
		if( strcmp( g_pGameDescription->mGameFile.GetBuffer(), gameList[x].gameFile ) == 0
			&& strcmp( fs_game, gameList[x].fs_game ) == 0 ){
			
			if( gameList[x].custom ){
				gtk_widget_hide( fs_game_label );
				gtk_widget_show( fs_game_entry );
			} 
			else
			{
				gtk_widget_hide( fs_game_entry );
				gtk_label_set_text( GTK_LABEL( fs_game_label ), gameList[x].fs_game );
				gtk_widget_show( fs_game_label );
			}
		}
	}

}

void ProjectSettings_dirbutton_clicked( GtkButton *button, gpointer user_data ){
	GtkWidget *dialog, *entry;
	const gchar *path;
	gchar *dir;

	dialog = GTK_WIDGET( user_data );
	entry = GTK_WIDGET( g_object_get_data( G_OBJECT( dialog ), "base" ) );
	path = gtk_entry_get_text( GTK_ENTRY( entry ) );

	dir = dir_dialog( dialog, _( "Select base directory" ), path );

	if( dir != NULL ) 
	{
		gtk_entry_set_text( GTK_ENTRY( entry ), dir );
		g_free( dir );
	}
}
void OnProjectViewSelChanged( GtkTreeSelection *treeselection, gpointer data )
{
	GtkWidget *change_button;
	GtkWidget *remove_button;
	GtkTreeIter iter;
	GtkTreeModel* model;
	GtkWidget* dialog; 

	dialog = GTK_WIDGET( data );

	change_button = GTK_WIDGET( g_object_get_data( G_OBJECT( dialog ), "change_button" ) );
	remove_button = GTK_WIDGET( g_object_get_data( G_OBJECT( dialog ), "remove_button" ) );

	if( gtk_tree_selection_get_selected( treeselection, &model, &iter ) ) 
	{
		gtk_widget_set_sensitive( GTK_WIDGET( change_button ), TRUE );
		gtk_widget_set_sensitive( GTK_WIDGET( remove_button ), TRUE );
	} else 
	{
		gtk_widget_set_sensitive( GTK_WIDGET( change_button ), FALSE );
		gtk_widget_set_sensitive( GTK_WIDGET( remove_button ), FALSE );
	}
}
static void OnDoProjectSettings_realize( GtkWidget *widget, gpointer data )
{
	OnProjectViewSelChanged( gtk_tree_view_get_selection( GTK_TREE_VIEW( widget ) ), data );
}

void DoProjectSettings(){
	GtkWidget *frame, *label, *vbox, *dialog, *content_area;
	GtkWidget *brush, *menu_frame, *hbox, *hbox2, *table, *add_button, *change_button, *remove_button;
	GtkWidget *scr, *menu_box, *entry, *button;
	GtkWidget *base, *game_select;
	GtkWidget *gamemode_combo, *fs_game_entry;
	GList *mod_list, *gamemode_list;
	GList *lst;
	GtkSizeGroup *button_group;
	gint response_id;
	GtkTreeSelection* selection;
	GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;

	dialog = gtk_dialog_new_with_buttons( _( "Project Settings" ), NULL, flags, NULL );
	gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "OK" ), GTK_RESPONSE_OK );
	gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "Cancel" ), GTK_RESPONSE_CANCEL );

	gtk_window_set_default_size( GTK_WINDOW( dialog ), 550, 400 );

	content_area = gtk_dialog_get_content_area( GTK_DIALOG( dialog ) );

	vbox = gtk_box_new( GTK_ORIENTATION_VERTICAL, 5 );
	gtk_container_add( GTK_CONTAINER( content_area ), vbox );
	gtk_container_set_border_width( GTK_CONTAINER( vbox ), 5 );
	gtk_widget_show( vbox );

	frame = gtk_frame_new( _( "Project settings" ) );
	gtk_box_pack_start( GTK_BOX( vbox ), frame, FALSE, FALSE, 5 );
	gtk_widget_set_hexpand( frame, TRUE );
	gtk_widget_set_vexpand( frame, FALSE );
	gtk_widget_show( frame );

	table = gtk_grid_new();
	gtk_container_add( GTK_CONTAINER( frame ), table );
	gtk_grid_set_row_spacing( GTK_GRID( table ), 5 );
	gtk_grid_set_column_spacing( GTK_GRID( table ), 5 );
	gtk_widget_show( table );

	label = gtk_label_new( _( "basepath" ) );
	gtk_grid_attach( GTK_GRID( table ), label, 0, 0, 1, 1 );
	gtk_widget_set_halign( label, GTK_ALIGN_START );
	gtk_widget_show( label );

	hbox2 = gtk_box_new( GTK_ORIENTATION_HORIZONTAL, 0 );
	gtk_grid_attach( GTK_GRID( table ), hbox2, 1, 0, 1, 1 );
	gtk_widget_show( hbox2 );

	base = gtk_entry_new();
	gtk_box_pack_start( GTK_BOX( hbox2 ), base, TRUE, TRUE, 0 );
	gtk_widget_set_hexpand( base, TRUE );
	gtk_widget_show( base );
	g_object_set_data( G_OBJECT( dialog ), "base", base );

	button = gtk_button_new_with_label( _( "..." ) );
	gtk_box_pack_start( GTK_BOX( hbox2 ), button, FALSE, FALSE, 0 );
	gtk_widget_set_hexpand( button, FALSE );
	gtk_widget_show( button );
	g_signal_connect( button, "clicked", G_CALLBACK( ProjectSettings_dirbutton_clicked ), dialog );

	const char *gamemode = ValueForKey( g_qeglobals.d_project_entity, "gamemode" );
	gamemode_list = newMappingModesListForGameFile( g_pGameDescription->mGameFile );
	if( gamemode_list )
	{
		label = gtk_label_new( _( "Mapping mode" ) );
		gtk_grid_attach( GTK_GRID( table ), label, 0, 1, 1, 1 );
		gtk_widget_set_halign( label, GTK_ALIGN_START );
		gtk_widget_show( label );

		gamemode_combo = gtk_combo_box_text_new();
		for( lst = gamemode_list; lst != NULL; lst = g_list_next( lst ) )
		{
			const gamemode_t *gamemode_x = (const gamemode_t *)lst->data;
			gtk_combo_box_text_append( GTK_COMBO_BOX_TEXT( gamemode_combo ), gamemode_x->mode, gamemode_x->name );
		}

		gtk_combo_box_set_active_id( GTK_COMBO_BOX( gamemode_combo ), gamemode );
		gtk_grid_attach( GTK_GRID( table ), gamemode_combo, 1, 1, 1, 1 );
		gtk_widget_set_hexpand( gamemode_combo, TRUE );
		gtk_widget_show( gamemode_combo );
	}

	label = gtk_label_new( _( "Game" ) );
	gtk_grid_attach( GTK_GRID( table ), label, 0, 2, 1, 1 );
	gtk_widget_set_halign( label, GTK_ALIGN_START );
	gtk_widget_show( label );

	const char *fs_game = ValueForKey( g_qeglobals.d_project_entity, "gamename" );
	mod_list = newModListForGameFile( g_pGameDescription->mGameFile );
	game_select = gtk_combo_box_text_new();
	qboolean base_mod = qtrue;
	for( lst = mod_list; lst != NULL; lst = g_list_next( lst ) )
	{
		const game_t *game_x = (const game_t *)lst->data;
		gtk_combo_box_text_append( GTK_COMBO_BOX_TEXT( game_select ), game_x->fs_game, game_x->name );
		if( fs_game && strlen( fs_game ) > 0 && strcmp( game_x->fs_game, fs_game ) == 0 )
		{
			gtk_combo_box_set_active_id( GTK_COMBO_BOX( game_select ), fs_game );
			base_mod = qfalse;
		}
	}
	if( base_mod )
	{
		for( lst = mod_list; lst != NULL; lst = g_list_next( lst ) )
		{
			const game_t *game_x = (const game_t *)lst->data;
			if( game_x->base )
			{
				gtk_combo_box_set_active_id( GTK_COMBO_BOX( game_select ), game_x->fs_game );
				break;
			}
		}
	}
	gtk_grid_attach( GTK_GRID( table ), game_select, 1, 2, 1, 1 );
	gtk_widget_set_hexpand( game_select, TRUE );
	gtk_widget_show( game_select );
	g_signal_connect( GTK_COMBO_BOX( game_select ), "changed", G_CALLBACK( OnSelchangeComboWhatgame ), dialog );
	g_signal_connect( G_OBJECT( dialog ), "realize", G_CALLBACK( OnDoProjectSettings_realize ), dialog );
	g_object_set_data( G_OBJECT( dialog ), "game_select", game_select );


	label = gtk_label_new( _( "fs_game" ) );
	gtk_grid_attach( GTK_GRID( table ), label, 0, 3, 1, 1 );
	gtk_widget_set_halign( label, GTK_ALIGN_START );
	gtk_widget_set_tooltip_text( label, _( "Directory name of the mod" ) );
	gtk_widget_show( label );

	fs_game_entry = entry = gtk_entry_new();
	gtk_grid_attach( GTK_GRID( table ), entry, 1, 3, 1, 1 );
	gtk_entry_set_text( GTK_ENTRY( entry ), fs_game );
	gtk_widget_set_hexpand( entry, TRUE );
//	gtk_widget_show( entry );
	g_object_set_data( G_OBJECT( dialog ), "fs_game_entry", entry );

	label = gtk_label_new( "" );
	gtk_grid_attach( GTK_GRID( table ), label, 1, 3, 1, 1 );
	gtk_widget_set_halign( label, GTK_ALIGN_START );
	gtk_widget_set_tooltip_text( label, _( "Directory name of the mod" ) );
//	gtk_widget_show( label );
	g_object_set_data( G_OBJECT( dialog ), "fs_game_label", label );

	
	menu_frame = gtk_frame_new( _( "Menu commands" ) );
	gtk_box_pack_start( GTK_BOX( vbox ), menu_frame, TRUE, TRUE, 0 );
	gtk_widget_set_hexpand( menu_frame, TRUE );
	gtk_widget_set_vexpand( menu_frame, TRUE );
	gtk_widget_show( menu_frame );

	menu_box = gtk_box_new( GTK_ORIENTATION_VERTICAL, 5 );
	gtk_container_add( GTK_CONTAINER( menu_frame ), menu_box );
	gtk_container_set_border_width( GTK_CONTAINER( menu_box ), 5 );
	gtk_widget_show( menu_box );

	scr = gtk_scrolled_window_new( (GtkAdjustment*)NULL, (GtkAdjustment*)NULL );
	gtk_box_pack_start( GTK_BOX( menu_box ), scr, TRUE, TRUE, 0 );
	gtk_container_set_border_width( GTK_CONTAINER( scr ), 5 );
	gtk_scrolled_window_set_policy( GTK_SCROLLED_WINDOW( scr ), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC );
	gtk_scrolled_window_set_shadow_type( GTK_SCROLLED_WINDOW( scr ), GTK_SHADOW_IN );
	gtk_widget_show( scr );
	
	{
		GtkListStore* store = gtk_list_store_new( 1, G_TYPE_STRING );

		GtkWidget* view = gtk_tree_view_new_with_model( GTK_TREE_MODEL( store ) );
		gtk_widget_set_hexpand( view, TRUE );
		gtk_widget_set_vexpand( view, TRUE );
		gtk_tree_view_set_headers_visible( GTK_TREE_VIEW( view ), FALSE );

		GtkCellRenderer* renderer = gtk_cell_renderer_text_new();
		GtkTreeViewColumn* column = gtk_tree_view_column_new_with_attributes( "", renderer, "text", 0, (char *) NULL );
		gtk_tree_view_append_column( GTK_TREE_VIEW( view ), column );

		selection = gtk_tree_view_get_selection( GTK_TREE_VIEW( view ) );
		gtk_tree_selection_set_mode( selection, GTK_SELECTION_BROWSE );
		g_signal_connect( selection, "changed", G_CALLBACK( OnProjectViewSelChanged ), dialog );

		g_object_set_data( G_OBJECT( dialog ), "view", view );
		g_object_set_data( G_OBJECT( dialog ), "bsp_commands", store );
		gtk_container_add( GTK_CONTAINER( scr ), view );

		g_object_unref( G_OBJECT( store ) );

		gtk_widget_show( view );

		column = gtk_tree_view_get_column( GTK_TREE_VIEW( view ), 0 );
		if( column ) {
			gint height = 0;
			gtk_tree_view_column_cell_get_size( column, NULL, NULL, NULL, NULL, &height );
			if( height > 0 ) {
				//show at least 5 rows
				gtk_scrolled_window_set_min_content_height( GTK_SCROLLED_WINDOW( scr ), height * 5 );
			}
		}

	}

	hbox = gtk_box_new( GTK_ORIENTATION_HORIZONTAL, 5 );
	gtk_box_pack_start( GTK_BOX( menu_box ), hbox, FALSE, FALSE, 0 );
	gtk_widget_show( hbox );

	add_button = gtk_button_new_with_label( _( "Add" ) );
	gtk_box_pack_start( GTK_BOX( hbox ), add_button, FALSE, FALSE, 0 );
	gtk_widget_show( add_button );
	g_signal_connect( add_button, "clicked", G_CALLBACK( project_add ), dialog );

	change_button = gtk_button_new_with_label( _( "Change" ) );
	gtk_box_pack_start( GTK_BOX( hbox ), change_button, FALSE, FALSE, 0 );
	gtk_widget_show( change_button );
	g_signal_connect( change_button, "clicked", G_CALLBACK( project_change ), dialog );
	g_object_set_data( G_OBJECT( dialog ), "change_button", change_button );

	remove_button = gtk_button_new_with_label( _( "Remove" ) );
	gtk_box_pack_start( GTK_BOX( hbox ), remove_button, FALSE, FALSE, 0 );
	gtk_widget_show( remove_button );
	g_signal_connect( remove_button, "clicked", G_CALLBACK( project_remove ), dialog );
	g_object_set_data( G_OBJECT( dialog ), "remove_button", remove_button );

	button_group = gtk_size_group_new( GTK_SIZE_GROUP_BOTH );
	gtk_size_group_add_widget( button_group, add_button );
	gtk_size_group_add_widget( button_group, change_button );
	gtk_size_group_add_widget( button_group, remove_button );
	g_object_unref( button_group );


	frame = gtk_frame_new( _( "Misc settings" ) );
	gtk_box_pack_start( GTK_BOX( vbox ), frame, FALSE, FALSE, 0 );
	gtk_widget_set_hexpand( frame, TRUE );
	gtk_widget_set_vexpand( frame, FALSE );
	gtk_widget_show( frame );

	brush = gtk_check_button_new_with_label( _( "Use brush primitives in MAP files" ) );
	gtk_container_add( GTK_CONTAINER( frame ), brush );
	gtk_container_set_border_width( GTK_CONTAINER( brush ), 5 );
	gtk_widget_set_tooltip_text( brush, _( "NOTE: experimental feature required by the texture tools plugin" ) );
	gtk_widget_set_vexpand( brush, FALSE );
	gtk_widget_show( brush );



	// Initialize fields
	gtk_entry_set_text( GTK_ENTRY( base ), ValueForKey( g_qeglobals.d_project_entity, "basepath" ) );
	UpdateBSPCommandList( dialog );
	gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( brush ), ( g_qeglobals.m_bBrushPrimitMode ) ? TRUE : FALSE );

	OnProjectViewSelChanged( selection, dialog );

	gtk_widget_show( dialog );

	g_pGameDescription->Dump();

	response_id = gtk_dialog_run( GTK_DIALOG( dialog ) );

	if( response_id == GTK_RESPONSE_OK ) 
	{
		char buf[1024];
		const char *r;
		char *w;
		const char *custom_fs_game, *selected_game, *new_fs_game;
		qboolean isbasegame;

		// convert path to unix format
		for ( r = gtk_entry_get_text( GTK_ENTRY( base ) ), w = buf; *r != '\0'; r++, w++ )
			*w = ( *r == '\\' ) ? '/' : *r;
		// add last slash
		if ( w != buf && *( w - 1 ) != '/' ) {
			*( w++ ) = '/';
		}
		// terminate string
		*w = '\0';
		SetKeyValue( g_qeglobals.d_project_entity, "basepath", buf );

		selected_game = gtk_combo_box_get_active_id( GTK_COMBO_BOX( game_select ) );
		custom_fs_game = gtk_entry_get_text( GTK_ENTRY( fs_game_entry ) );

		isbasegame = qfalse;
		new_fs_game = NULL;

		if( !selected_game )
		{
			isbasegame = qtrue; //should never happen that none is selected
		} else {
			for( lst = mod_list; lst != NULL; lst = g_list_next( lst ) )
			{
				const game_t *game_x = (const game_t *)lst->data;
				 if( strcmp( game_x->fs_game, selected_game ) == 0 )
				 {
					if( game_x->base ) 
					{
						isbasegame = qtrue;
					} else 
					if( game_x->custom ) 
					{
						if( !custom_fs_game || strlen( custom_fs_game ) == 0 )
						{
							isbasegame = qtrue;
						} else
						{
							new_fs_game = custom_fs_game;
						}
					} else 
					{
						new_fs_game = game_x->fs_game;
					}
				}
			}
		}
		if( new_fs_game == NULL ) {
			isbasegame = qtrue;
		}
		if( isbasegame ) {
			DeleteKey( g_qeglobals.d_project_entity, "gamename" );
		} else {
			SetKeyValue( g_qeglobals.d_project_entity, "gamename", new_fs_game );
		}

		if( gamemode_list )
		{
			const char *selected_mode;
			const char *new_mode;
			
			selected_mode = gtk_combo_box_get_active_id( GTK_COMBO_BOX( gamemode_combo ) );
			new_mode = NULL;

			if( !selected_mode )
			{
				new_mode = NULL;
			} else {
				for( lst = gamemode_list; lst != NULL; lst = g_list_next( lst ) )
				{
					const gamemode_t *gamemode_x = (const gamemode_t *)lst->data;
					if( strcmp( gamemode_x->mode, selected_mode ) == 0 )
					{
						new_mode = selected_mode;
						break;
					}
				}
			}
			if( !new_mode ) 
			{
				for( lst = gamemode_list; lst != NULL; lst = g_list_next( lst ) )
				{
					const gamemode_t *gamemode_x = (const gamemode_t *)lst->data;
					new_mode = gamemode_x->mode;
					break;
				}
			}
			if( new_mode ) {
				SetKeyValue( g_qeglobals.d_project_entity, "gamemode", new_mode );
			}
		}

		if ( gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( brush ) ) ) {
			g_qeglobals.m_bBrushPrimitMode = TRUE;
		}
		else{
			g_qeglobals.m_bBrushPrimitMode = FALSE;
		}

		SetKeyValue( g_qeglobals.d_project_entity, "brush_primit", ( g_qeglobals.m_bBrushPrimitMode ? "1" : "0" ) );

		QE_SaveProject( g_PrefsDlg.m_strLastProject.GetBuffer() );
	}

	g_list_free( mod_list );
	g_list_free( gamemode_list );

	gtk_widget_destroy( dialog );
}

// =============================================================================
// MapInfo dialog

void DoMapInfo(){
	static GtkWidget *dialog;
	GtkWidget *vbox, *table, *label, *scr;
	GtkWidget *brushes_label, *entities_label, *net_label, *content_area;
	gint response_id;
	GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;

	if ( dialog != NULL ) {
		return;
	}

	dialog = gtk_dialog_new_with_buttons( _( "Map Info" ), NULL, flags, NULL );
	gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "OK" ), GTK_RESPONSE_OK );

	load_window_pos( dialog, g_PrefsDlg.mWindowInfo.posMapInfoWnd );

	content_area = gtk_dialog_get_content_area( GTK_DIALOG( dialog ) );

	vbox = gtk_box_new( GTK_ORIENTATION_VERTICAL, 5 );
	gtk_container_add( GTK_CONTAINER( content_area ), vbox );
	gtk_container_set_border_width( GTK_CONTAINER( vbox ), 5 );
	gtk_widget_show( vbox );

	table = gtk_grid_new();
	gtk_box_pack_start( GTK_BOX( vbox ), table, FALSE, FALSE, 0 );
	gtk_grid_set_row_spacing( GTK_GRID( table ), 5 );
	gtk_grid_set_column_spacing( GTK_GRID( table ), 5 );
	gtk_widget_show( table );

	label = gtk_label_new( _( "Total Brushes" ) );
	gtk_grid_attach( GTK_GRID( table ), label, 0, 0, 1, 1 );
	gtk_widget_set_halign( label, GTK_ALIGN_START );
	gtk_widget_show( label );

	brushes_label = gtk_label_new( "" );
	gtk_grid_attach( GTK_GRID( table ), brushes_label, 1, 0, 1, 1 );
	gtk_widget_set_halign( brushes_label, GTK_ALIGN_START );
	gtk_widget_show( brushes_label );
	g_object_set( brushes_label, "xalign", 1.0, NULL );

	label = gtk_label_new( _( "Total Entities" ) );
	gtk_grid_attach( GTK_GRID( table ), label, 0, 1, 1, 1 );
	gtk_widget_set_halign( label, GTK_ALIGN_START );
	gtk_widget_show( label );

	entities_label = gtk_label_new( "" );
	gtk_grid_attach( GTK_GRID( table ), entities_label, 1, 1, 1, 1 );
	gtk_widget_set_halign( entities_label, GTK_ALIGN_START );
	gtk_widget_show( entities_label );
	g_object_set( entities_label, "xalign", 1.0, NULL );

	label = gtk_label_new( _( "Net brush count (non entity)" ) );
	gtk_grid_attach( GTK_GRID( table ), label, 0, 2, 1, 1 );
	gtk_widget_set_halign( label, GTK_ALIGN_START );
	gtk_widget_show( label );

	net_label = gtk_label_new( "" );
	gtk_grid_attach( GTK_GRID( table ), net_label, 1, 2, 1, 1 );
	gtk_widget_set_halign( net_label, GTK_ALIGN_START );
	gtk_widget_show( net_label );
	g_object_set( net_label, "xalign", 1.0, NULL );

	label = gtk_label_new( _( "Entity breakdown" ) );
	gtk_box_pack_start( GTK_BOX( vbox ), label, FALSE, FALSE, 0 );
	gtk_widget_set_halign( label, GTK_ALIGN_START );
	gtk_widget_show( label );

	scr = gtk_scrolled_window_new( (GtkAdjustment*)NULL, (GtkAdjustment*)NULL ); //
	gtk_widget_set_hexpand( scr, TRUE );
	gtk_widget_set_vexpand( scr, TRUE );
	gtk_box_pack_start( GTK_BOX( vbox ), scr, TRUE, TRUE, 0 );
	gtk_container_set_border_width( GTK_CONTAINER( scr ), 5 );
	gtk_scrolled_window_set_policy( GTK_SCROLLED_WINDOW( scr ), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC );
	gtk_scrolled_window_set_shadow_type( GTK_SCROLLED_WINDOW( scr ), GTK_SHADOW_IN );
	gtk_widget_show( scr );

	GtkListStore* store = gtk_list_store_new( 2, G_TYPE_STRING, G_TYPE_STRING );

	{
		GtkWidget* view = gtk_tree_view_new_with_model( GTK_TREE_MODEL( store ) );
		gtk_widget_set_hexpand( view, TRUE );
		gtk_widget_set_vexpand( view, TRUE );
		gtk_tree_view_set_headers_clickable( GTK_TREE_VIEW( view ), TRUE );

		{
			GtkCellRenderer* renderer = gtk_cell_renderer_text_new();
			g_object_set( renderer, "xalign", 1.0, NULL );
			GtkTreeViewColumn* column = gtk_tree_view_column_new_with_attributes( _( "Count" ), renderer, "text", 0, (char *) NULL );
			gtk_tree_view_append_column( GTK_TREE_VIEW( view ), column );
			gtk_tree_view_column_set_sort_column_id( column, 0 );
		}

		{
			GtkCellRenderer* renderer = gtk_cell_renderer_text_new();
			GtkTreeViewColumn* column = gtk_tree_view_column_new_with_attributes( _( "Entity" ), renderer, "text", 1, (char *) NULL );			
			gtk_tree_view_append_column( GTK_TREE_VIEW( view ), column );
			gtk_tree_view_column_set_sort_column_id( column, 1 );
		}

		gtk_container_add( GTK_CONTAINER( scr ), view );

		g_object_unref( G_OBJECT( store ) );
		gtk_widget_show( view );

		GtkTreeViewColumn * column = gtk_tree_view_get_column( GTK_TREE_VIEW( view ), 0 );
		if( column ) {
			gint height = 0;
			gtk_tree_view_column_cell_get_size( column, NULL, NULL, NULL, NULL, &height );
			if( height > 0 ) {
				//show at least 5 rows plus header
				gtk_scrolled_window_set_min_content_height( GTK_SCROLLED_WINDOW( scr ), height * 6 );
				//gtk_widget_set_size_request( view, -1, height * 6 );
			}
		}
	}

	// Initialize fields
	int TotalBrushes = 0, TotalEntities = 0, Net = 0;

	for ( brush_t* pBrush = active_brushes.next; pBrush != &active_brushes; pBrush = pBrush->next )
	{
		TotalBrushes++;
		if ( pBrush->owner == world_entity ) {
			Net++;
		}
	}

	typedef struct
	{
		const char *name;
		int count;
	} map_t;

	GSList *l, *entitymap = NULL;
	map_t *entry;

	for ( entity_t* pEntity = entities.next; pEntity != &entities; pEntity = pEntity->next )
	{
		TotalEntities++;
		bool add = true;

		for ( l = entitymap; l; l = g_slist_next( l ) )
		{
			entry = (map_t*)l->data;

			if ( strcmp( entry->name, pEntity->eclass->name ) == 0 ) {
				entry->count++;
				add = false;
				break;
			}
		}

		if ( add ) {
			entry = (map_t*)g_malloc( sizeof( map_t ) );
			entry->name = pEntity->eclass->name;
			entry->count = 1;
			entitymap = g_slist_append( entitymap, entry );
		}
	}

	while ( entitymap )
	{
		entry = (map_t*)entitymap->data;
		char tmp[16];
		snprintf( tmp, sizeof( tmp ), "%d", entry->count );
		GtkTreeIter iter;
		gtk_list_store_append( GTK_LIST_STORE( store ), &iter );
		gtk_list_store_set( GTK_LIST_STORE( store ), &iter, 0, tmp, 1, entry->name, -1 );
		g_free( entry );
		entitymap = g_slist_remove( entitymap, entry );
	}

	char tmp[16];
	snprintf( tmp, sizeof( tmp ), "%d", TotalBrushes );
	gtk_label_set_text( GTK_LABEL( brushes_label ), tmp );
	snprintf( tmp, sizeof( tmp ), "%d", TotalEntities );
	gtk_label_set_text( GTK_LABEL( entities_label ), tmp );
	snprintf( tmp, sizeof( tmp ), "%d", Net );
	gtk_label_set_text( GTK_LABEL( net_label ), tmp );

	response_id = gtk_dialog_run( GTK_DIALOG( dialog ) );

	gtk_widget_destroy( dialog );
	dialog = NULL;
}

// =============================================================================
// Entity List dialog

static void entitylist_select( GtkWidget *widget, gpointer data ){
	GtkTreeView* view = GTK_TREE_VIEW( g_object_get_data( G_OBJECT( data ), "entities" ) );

	GtkTreeSelection* selection = gtk_tree_view_get_selection( view );

	GtkTreeModel* model;
	GtkTreeIter selected;
	if ( gtk_tree_selection_get_selected( selection, &model, &selected ) ) {
		entity_t* pEntity;
		gtk_tree_model_get( model, &selected, 1, &pEntity, -1 );

		if ( pEntity ) {
			for ( epair_t* pEpair = pEntity->epairs; pEpair; pEpair = pEpair->next )
			{
				Select_Deselect();
				Select_Brush( pEntity->brushes.onext );
				Sys_UpdateWindows( W_ALL );
			}
		}
	}
}

static gint entitylist_click( GtkWidget *widget, GdkEventButton *event, gpointer data ){
	if ( event->type == GDK_2BUTTON_PRESS ) {
		entitylist_select( NULL, data );
		return TRUE;
	}
	return FALSE;
}

static void entitylist_selection_changed( GtkTreeSelection* selection, gpointer data ){
	GtkListStore* store = GTK_LIST_STORE( g_object_get_data( G_OBJECT( data ), "keyvalues" ) );

	gtk_list_store_clear( store );

	GtkTreeModel* model;
	GtkTreeIter selected;
	if ( gtk_tree_selection_get_selected( selection, &model, &selected ) ) {
		entity_t* pEntity;
		gtk_tree_model_get( model, &selected, 1, &pEntity, -1 );

		if ( pEntity ) {
			for ( epair_t* pEpair = pEntity->epairs; pEpair; pEpair = pEpair->next )
			{
				GtkTreeIter appended;
				gtk_list_store_append( store, &appended );
				gtk_list_store_set( store, &appended, 0, pEpair->key, 1, pEpair->value, -1 );
			}
		}
	}
}

void DoEntityList(){
	static GtkWidget *dialog;
	GtkWidget *vbox, *vbox2, *hbox, *hbox2, *button, *scr, *content_area;
	gint response_id;
	GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;

	if ( dialog != NULL ) {
		return;
	}

	dialog = gtk_dialog_new_with_buttons( _( "Entities" ), NULL, flags, NULL );
	gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "OK" ), GTK_RESPONSE_OK );

	content_area = gtk_dialog_get_content_area( GTK_DIALOG( dialog ) );

	load_window_pos( dialog, g_PrefsDlg.mWindowInfo.posEntityInfoWnd );

	hbox = gtk_box_new( GTK_ORIENTATION_HORIZONTAL, 5 );
	gtk_container_add( GTK_CONTAINER( content_area ), hbox );
	gtk_container_set_border_width( GTK_CONTAINER( hbox ), 5 );
	gtk_widget_show( hbox );

	vbox2 = gtk_box_new( GTK_ORIENTATION_VERTICAL, 5 );
	gtk_box_pack_start( GTK_BOX( hbox ), vbox2, TRUE, TRUE, 0 );
	gtk_widget_show( vbox2 );

	scr = gtk_scrolled_window_new( (GtkAdjustment*)NULL, (GtkAdjustment*)NULL );
	gtk_box_pack_start( GTK_BOX( vbox2 ), scr, TRUE, TRUE, 0 );
	gtk_scrolled_window_set_policy( GTK_SCROLLED_WINDOW( scr ), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC );
	gtk_scrolled_window_set_shadow_type( GTK_SCROLLED_WINDOW( scr ), GTK_SHADOW_IN );
	gtk_widget_set_hexpand( scr, TRUE );
	gtk_widget_set_vexpand( scr, TRUE );
	gtk_widget_show( scr );

	{
		GtkTreeStore* store = gtk_tree_store_new( 2, G_TYPE_STRING, G_TYPE_POINTER );

		GtkWidget* view = gtk_tree_view_new_with_model( GTK_TREE_MODEL( store ) );
		g_signal_connect( view, "button_press_event", G_CALLBACK( entitylist_click ), dialog );
		gtk_tree_view_set_headers_visible( GTK_TREE_VIEW( view ), FALSE );

		{
			GtkCellRenderer* renderer = gtk_cell_renderer_text_new();
			GtkTreeViewColumn* column = gtk_tree_view_column_new_with_attributes( "", renderer, "text", 0, (char *) NULL );
			gtk_tree_view_append_column( GTK_TREE_VIEW( view ), column );
		}

		{
			GtkTreeSelection* selection = gtk_tree_view_get_selection( GTK_TREE_VIEW( view ) );
			g_signal_connect( selection, "changed", G_CALLBACK( entitylist_selection_changed ), dialog );
		}

		gtk_container_add( GTK_CONTAINER( scr ), view );
		g_object_set_data( G_OBJECT( dialog ), "entities", view );

		gtk_widget_show( view );

		{
			{
				GtkTreeIter child;
				gtk_tree_store_append( store, &child, NULL );
				gtk_tree_store_set( store, &child, 0, world_entity->eclass->name, 1, world_entity, -1 );
			}

			GSList *l, *entitymap = NULL;
			typedef struct
			{
				GtkTreeIter node;
				const char *name;
			} map_t;
			map_t *entry;

			for ( entity_t* pEntity = entities.next; pEntity != &entities; pEntity = pEntity->next )
			{
				GtkTreeIter parent;
				bool found = false;

				for ( l = entitymap; l; l = g_slist_next( l ) )
				{
					entry = (map_t*)l->data;

					if ( strcmp( entry->name, pEntity->eclass->name ) == 0 ) {
						parent = entry->node;
						found = true;
						break;
					}
				}

				if ( !found ) {
					gtk_tree_store_append( store, &parent, NULL );
					gtk_tree_store_set( store, &parent, 0, pEntity->eclass->name, 1, NULL, -1 );

					entry = (map_t*)g_malloc( sizeof( map_t ) );
					entitymap = g_slist_append( entitymap, entry );
					entry->name = pEntity->eclass->name;
					entry->node = parent;
				}

				GtkTreeIter child;
				gtk_tree_store_append( store, &child, &parent );
				gtk_tree_store_set( store, &child, 0, pEntity->eclass->name, 1, pEntity, -1 );
			}

			while ( entitymap )
			{
				g_free( entitymap->data );
				entitymap = g_slist_remove( entitymap, entitymap->data );
			}
		}

		g_object_unref( G_OBJECT( store ) );
	}

	vbox = gtk_box_new( GTK_ORIENTATION_VERTICAL, 5 );
	gtk_box_pack_start( GTK_BOX( hbox ), vbox, TRUE, TRUE, 0 );
	gtk_widget_show( vbox );

	scr = gtk_scrolled_window_new( (GtkAdjustment*)NULL, (GtkAdjustment*)NULL );
	gtk_box_pack_start( GTK_BOX( vbox ), scr, TRUE, TRUE, 0 );
	gtk_scrolled_window_set_policy( GTK_SCROLLED_WINDOW( scr ), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC );
	gtk_scrolled_window_set_shadow_type( GTK_SCROLLED_WINDOW( scr ), GTK_SHADOW_IN );
	gtk_widget_set_hexpand( scr, TRUE );
	gtk_widget_set_vexpand( scr, TRUE );
	gtk_widget_show( scr );

	{
		GtkListStore* store = gtk_list_store_new( 2, G_TYPE_STRING, G_TYPE_STRING );

		GtkWidget* view = gtk_tree_view_new_with_model( GTK_TREE_MODEL( store ) );

		{
			GtkCellRenderer* renderer = gtk_cell_renderer_text_new();
			GtkTreeViewColumn* column = gtk_tree_view_column_new_with_attributes( _( "Key" ), renderer, "text", 0, (char *) NULL );
			gtk_tree_view_append_column( GTK_TREE_VIEW( view ), column );
		}

		{
			GtkCellRenderer* renderer = gtk_cell_renderer_text_new();
			GtkTreeViewColumn* column = gtk_tree_view_column_new_with_attributes( _( "Value" ), renderer, "text", 1, (char *) NULL );
			gtk_tree_view_append_column( GTK_TREE_VIEW( view ), column );
		}

		gtk_widget_set_hexpand( view, TRUE );
		gtk_widget_set_vexpand( view, TRUE );
		gtk_widget_show( view );

		g_object_set_data( G_OBJECT( dialog ), "keyvalues", store );
		gtk_container_add( GTK_CONTAINER( scr ), view );

		g_object_unref( G_OBJECT( store ) );
	}

	hbox2 = gtk_box_new( GTK_ORIENTATION_HORIZONTAL, 5 );
	gtk_box_pack_start( GTK_BOX( vbox ), hbox2, FALSE, FALSE, 0 );
	gtk_widget_show( hbox2 );

	button = gtk_button_new_with_label( _( "Select" ) );
	gtk_box_pack_start( GTK_BOX( hbox2 ), button, FALSE, FALSE, 0 );
	g_signal_connect( button, "clicked", G_CALLBACK( entitylist_select ), dialog );
	gtk_widget_set_hexpand( button, FALSE );
	gtk_widget_set_vexpand( button, FALSE );
	gtk_widget_show( button );


	gtk_widget_show( dialog );

	response_id = gtk_dialog_run( GTK_DIALOG( dialog ) );

	save_window_pos( dialog, g_PrefsDlg.mWindowInfo.posMapInfoWnd );

	gtk_widget_destroy( dialog );

	dialog = NULL;
}

// =============================================================================
// Rotate dialog

static void rotatedialog_response( GtkWidget *widget, gint response_id, gpointer data ){
	GtkSpinButton *spin;
	float f;

	if ( response_id == GTK_RESPONSE_CANCEL )
    {
		gtk_widget_destroy( GTK_WIDGET( widget ) );
		return;
	}
	if ( !( response_id == GTK_RESPONSE_OK || response_id == GTK_RESPONSE_APPLY ) )
    {
		return;
	}
	spin = GTK_SPIN_BUTTON( g_object_get_data( G_OBJECT( data ), "x" ) );
	f = gtk_spin_button_get_value( spin );
	if ( f != 0.0 ) {
		Select_RotateAxis( 0, f );
	}
	gtk_spin_button_set_value( GTK_SPIN_BUTTON( spin ), 0.0f ); // reset to 0 on Apply

	spin = GTK_SPIN_BUTTON( g_object_get_data( G_OBJECT( data ), "y" ) );
	f = gtk_spin_button_get_value( spin );
	if ( f != 0.0 ) {
		Select_RotateAxis( 1, f );
	}
	gtk_spin_button_set_value( GTK_SPIN_BUTTON( spin ), 0.0f );

	spin = GTK_SPIN_BUTTON( g_object_get_data( G_OBJECT( data ), "z" ) );
	f = gtk_spin_button_get_value( spin );
	if ( f != 0.0 ) {
		Select_RotateAxis( 2, f );
	}
	gtk_spin_button_set_value( GTK_SPIN_BUTTON( spin ), 0.0f );

	if ( response_id == GTK_RESPONSE_OK )
    {
		gtk_widget_destroy( GTK_WIDGET( widget ) );
	}
}

void DoRotateDlg(){
	GtkWidget *dialog, *vbox1, *table, *label;
	GtkWidget *x, *y, *z, *content_area;
	GtkAdjustment *adj;
	GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;

	dialog = gtk_dialog_new_with_buttons( _( "Arbitrary rotation" ), NULL, flags, NULL );
	gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "OK" ), GTK_RESPONSE_OK );
	gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "Cancel" ), GTK_RESPONSE_CANCEL );
	gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "Apply" ), GTK_RESPONSE_APPLY );

	content_area = gtk_dialog_get_content_area( GTK_DIALOG( dialog ) );

	vbox1 = gtk_box_new( GTK_ORIENTATION_VERTICAL, 5 );
	gtk_container_add( GTK_CONTAINER( content_area ), vbox1 );
	gtk_container_set_border_width( GTK_CONTAINER( vbox1 ), 5 );
	gtk_widget_show( vbox1 );

	label = gtk_label_new( _( "Rotate the selection around an axis:" ) );
	gtk_widget_set_halign( label, GTK_ALIGN_START );
	gtk_box_pack_start( GTK_BOX( vbox1 ), label, FALSE, FALSE, 0 );
	gtk_widget_show( label );

	table = gtk_grid_new();
	gtk_box_pack_start( GTK_BOX( vbox1 ), table, TRUE, TRUE, 0 );
	gtk_grid_set_row_spacing( GTK_GRID( table ), 5 );
	gtk_grid_set_column_spacing( GTK_GRID( table ), 5 );
	gtk_widget_show( table );

	label = gtk_label_new( _( "X:" ) );
	gtk_widget_set_halign( label, GTK_ALIGN_START );
	gtk_grid_attach( GTK_GRID( table ), label, 0, 0, 1, 1 );
	gtk_widget_show( label );

	adj = gtk_adjustment_new( 0, -359, 359, 1, 10, 0 );
	x = gtk_spin_button_new( GTK_ADJUSTMENT( adj ), 1, 0 );
	gtk_grid_attach( GTK_GRID( table ), x, 1, 0, 1, 1 );
	gtk_spin_button_set_wrap( GTK_SPIN_BUTTON( x ), TRUE );
	gtk_spin_button_set_numeric( GTK_SPIN_BUTTON( x ), TRUE );
	gtk_widget_set_hexpand( x, TRUE );
	gtk_widget_show( x );
	g_object_set_data( G_OBJECT( dialog ), "x", x );
	g_object_set( x, "xalign", 1.0, NULL ); //right align numbers

	label = gtk_label_new( _( "\302\260" ) );
	gtk_grid_attach( GTK_GRID( table ), label, 2, 0, 1, 1 );
	gtk_widget_set_tooltip_text( label, _( "Degree" ) );
	gtk_widget_show( label );

	label = gtk_label_new( _( "Y:" ) );
	gtk_grid_attach( GTK_GRID( table ), label, 0, 1, 1, 1 );
	gtk_widget_set_halign( label, GTK_ALIGN_START );
	gtk_widget_show( label );

	adj = gtk_adjustment_new( 0, -359, 359, 1, 10, 0 );
	y = gtk_spin_button_new( GTK_ADJUSTMENT( adj ), 1, 0 );
	gtk_grid_attach( GTK_GRID( table ), y, 1, 1, 1, 1 );
	gtk_spin_button_set_wrap( GTK_SPIN_BUTTON( y ), TRUE );
	gtk_spin_button_set_numeric( GTK_SPIN_BUTTON( y ), TRUE );
	gtk_widget_set_hexpand( y, TRUE );
	gtk_widget_show( y );
	g_object_set_data( G_OBJECT( dialog ), "y", y );
	g_object_set( y, "xalign", 1.0, NULL );

	label = gtk_label_new( _( "\302\260" ) );
	gtk_grid_attach( GTK_GRID( table ), label, 2, 1, 1, 1 );
	gtk_widget_set_tooltip_text( label, _( "Degree" ) );
	gtk_widget_show( label );

	label = gtk_label_new( _( "Z:" ) );
	gtk_grid_attach( GTK_GRID( table ), label, 0, 2, 1, 1 );
	gtk_widget_set_halign( label, GTK_ALIGN_START );
	gtk_widget_show( label );

	adj = gtk_adjustment_new( 0, -359, 359, 1, 10, 0 );
	z = gtk_spin_button_new( GTK_ADJUSTMENT( adj ), 1, 0 );
	gtk_grid_attach( GTK_GRID( table ), z, 1, 2, 1, 1 );
	gtk_spin_button_set_wrap( GTK_SPIN_BUTTON( z ), TRUE );
	gtk_spin_button_set_numeric( GTK_SPIN_BUTTON( z ), TRUE );
	gtk_widget_set_hexpand( z, TRUE );
	gtk_widget_show( z );
	g_object_set_data( G_OBJECT( dialog ), "z", z );
	g_object_set( z, "xalign", 1.0, NULL );

	label = gtk_label_new( _( "\302\260" ) );
	gtk_grid_attach( GTK_GRID( table ), label, 2, 2, 1, 1 );
	gtk_widget_set_tooltip_text( label, _( "Degree" ) );
	gtk_widget_show( label );

	g_signal_connect( dialog, "response", G_CALLBACK( rotatedialog_response ), dialog );

	gtk_widget_show( dialog );
}

// =============================================================================
// Gamma dialog

void DoGamma(){
	GtkWidget *dialog, *vbox, *label, *spin, *content_area;
	gint response_id;
	GtkAdjustment *adj;
	GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;

	dialog = gtk_dialog_new_with_buttons( _( "Gamma" ), NULL, flags, NULL );
	gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "OK" ), GTK_RESPONSE_OK );
	gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "Cancel" ), GTK_RESPONSE_CANCEL );

	content_area = gtk_dialog_get_content_area( GTK_DIALOG( dialog ) );

	vbox = gtk_box_new( GTK_ORIENTATION_VERTICAL, 5 );
	gtk_container_add( GTK_CONTAINER( content_area ), vbox );
	gtk_container_set_border_width( GTK_CONTAINER( vbox ), 5 );
	gtk_widget_show( vbox );

	label = gtk_label_new( _( "0.0 is brightest\n1.0 is darkest" ) );
	gtk_widget_set_halign( label, GTK_ALIGN_START );
	gtk_box_pack_start( GTK_BOX( vbox ), label, TRUE, TRUE, 0 );
	gtk_widget_show( label );

	label = gtk_label_new( _( "You must restart for the\nsettings to take effect" ) );
	gtk_widget_set_halign( label, GTK_ALIGN_START );
	gtk_box_pack_start( GTK_BOX( vbox ), label, TRUE, TRUE, 0 );
	gtk_widget_show( label );

	adj = gtk_adjustment_new( 1, 0, 1, 0.1, 0.01, 0 );
	spin = gtk_spin_button_new( GTK_ADJUSTMENT( adj ), 1, 2 );
	gtk_box_pack_start( GTK_BOX( vbox ), spin, TRUE, TRUE, 0 );
	gtk_spin_button_set_wrap( GTK_SPIN_BUTTON( spin ), FALSE );
	gtk_spin_button_set_numeric( GTK_SPIN_BUTTON( spin ), TRUE );
	gtk_widget_set_hexpand( spin, TRUE );
	gtk_widget_show( spin );
	g_object_set( spin, "xalign", 1.0, NULL );

	// Initialize dialog
	gtk_spin_button_set_value( GTK_SPIN_BUTTON( spin ), g_qeglobals.d_savedinfo.fGamma );

	gtk_widget_show( dialog );

	response_id = gtk_dialog_run( GTK_DIALOG( dialog ) );

	if( response_id == GTK_RESPONSE_OK ) 
	{
		g_qeglobals.d_savedinfo.fGamma = gtk_spin_button_get_value( GTK_SPIN_BUTTON( spin ) );
	}

	gtk_widget_destroy( dialog );
}

// =============================================================================
// Find Brush Dialog

// helper function to walk through the active brushes only and drop the regioned out ones
bool WalkRegionBrush( brush_t **b, entity_t *e ){
	brush_t *b2;
	do
	{
		for ( b2 = active_brushes.next ; b2 != &active_brushes ; b2 = b2->next )
		{
			if ( b2 == *b ) {
				break; // this is an active brush
			}
		}
		if ( b2 == &active_brushes ) {
			// this is a regioned out brush
			*b = ( *b )->onext;
			if ( *b == &e->brushes ) {
				Sys_Status( "No such brush", 0 );
				return false;
			}
		}
	} while ( b2 == &active_brushes );
	return true;
}

void SelectBrush( int entitynum, int brushnum ){
	entity_t *e;
	brush_t *b;
	int i;

	// making this work when regioning is on too

	if ( entitynum == 0 ) {
		e = world_entity;
	}
	else
	{
		e = entities.next;
		while ( --entitynum )
		{
			e = e->next;
			if ( e == &entities ) {
				Sys_Status( "No such entity", 0 );
				return;
			}
			if ( region_active ) {
				// we need to make sure we walk to the next 'active' entity to have a valid --entitynum
				// that is, find a brush that belongs to this entity in the active brushes
				do
				{
					for ( b = active_brushes.next ; b != &active_brushes ; b = b->next )
					{
						if ( b->owner == e ) {
							break; // this is an active entity
						}
					}
					if ( b == &active_brushes ) {
						// this is a regioned out entity
						e = e->next;
						// don't walk past the end either
						if ( e == &entities ) {
							Sys_Status( "No such entity", 0 );
							return;
						}
					}
				} while ( b == &active_brushes );
			}
		}
	}

	b = e->brushes.onext;
	if ( b == &e->brushes ) {
		Sys_Status( "No such brush", 0 );
		return;
	}
	if ( region_active ) {
		if ( !WalkRegionBrush( &b, e ) ) {
			return;
		}
	}

	while ( brushnum-- )
	{
		b = b->onext;
		if ( b == &e->brushes ) {
			Sys_Status( "No such brush", 0 );
			return;
		}
		if ( region_active ) {
			if ( !WalkRegionBrush( &b, e ) ) {
				return;
			}
		}
	}

	Brush_RemoveFromList( b );
	Brush_AddToList( b, &selected_brushes );

	Sys_UpdateWindows( W_ALL );
	for ( i = 0; i < 3; i++ )
	{
		if ( g_pParentWnd->GetXYWnd() ) {
			g_pParentWnd->GetXYWnd()->GetOrigin()[i] = ( b->mins[i] + b->maxs[i] ) / 2;
		}

		if ( g_pParentWnd->GetXZWnd() ) {
			g_pParentWnd->GetXZWnd()->GetOrigin()[i] = ( b->mins[i] + b->maxs[i] ) / 2;
		}

		if ( g_pParentWnd->GetYZWnd() ) {
			g_pParentWnd->GetYZWnd()->GetOrigin()[i] = ( b->mins[i] + b->maxs[i] ) / 2;
		}
	}

	Sys_Status( "Selected", 0 );
}

static void GetSelectionIndex( int *ent, int *brush ){
	brush_t *b, *b2;
	entity_t *entity;

	*ent = *brush = 0;

	b = selected_brushes.next;
	if ( b == &selected_brushes ) {
		return;
	}

	// find entity
	if ( b->owner != world_entity ) {
		( *ent )++;
		for ( entity = entities.next; entity != &entities; entity = entity->next, ( *ent )++ )
			;
	}

	// find brush
	for ( b2 = b->owner->brushes.onext; b2 != b && b2 != &b->owner->brushes; b2 = b2->onext, ( *brush )++ )
		;
}
static void findbrushdialog_response( GtkWidget *widget, gint response_id, gpointer data ){
	int ent_num;
	int brush_num;
	GtkSpinButton *spin;

	if ( response_id == GTK_RESPONSE_CANCEL )
    {
		gtk_widget_destroy( GTK_WIDGET( widget ) );
		return;
	}
	if ( !( response_id == GTK_RESPONSE_OK || response_id == GTK_RESPONSE_APPLY ) )
    {
		return;
	}

	spin = GTK_SPIN_BUTTON( g_object_get_data( G_OBJECT( data ), "entity-spin" ) );
	ent_num = gtk_spin_button_get_value_as_int( spin );

	spin = GTK_SPIN_BUTTON( g_object_get_data( G_OBJECT( data ), "brush-spin" ) );
	brush_num = gtk_spin_button_get_value_as_int( spin );

	SelectBrush( ent_num, brush_num );

	if ( response_id == GTK_RESPONSE_OK )
    {
		gtk_widget_destroy( GTK_WIDGET( widget ) );
	}
}

void DoFind(){
	GtkWidget *dialog, *vbox, *table, *label, *entity, *brush, *content_area, *spin;
	GtkAdjustment *adj;
	int ent, br;
	GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;

	dialog = gtk_dialog_new_with_buttons( _( "Find Brush" ), NULL, flags, NULL );
	gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "OK" ), GTK_RESPONSE_OK );
	gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "Cancel" ), GTK_RESPONSE_CANCEL );

	content_area = gtk_dialog_get_content_area( GTK_DIALOG( dialog ) );

	vbox = gtk_box_new( GTK_ORIENTATION_VERTICAL, 5 );
	gtk_container_add( GTK_CONTAINER( content_area ), vbox );
	gtk_container_set_border_width( GTK_CONTAINER( vbox ), 5 );
	gtk_widget_show( vbox );

	table = gtk_grid_new();
	gtk_box_pack_start( GTK_BOX( vbox ), table, TRUE, TRUE, 0 );
	gtk_grid_set_row_spacing( GTK_GRID( table ), 5 );
	gtk_grid_set_column_spacing( GTK_GRID( table ), 5 );
	gtk_widget_show( table );

	label = gtk_label_new( _( "Entity number" ) );
	gtk_widget_set_halign( label, GTK_ALIGN_START );
	gtk_grid_attach( GTK_GRID( table ), label, 0, 0, 1, 1 );
	gtk_widget_show( label );

	label = gtk_label_new( _( "Brush number" ) );
	gtk_widget_set_halign( label, GTK_ALIGN_START );
	gtk_grid_attach( GTK_GRID( table ), label, 0, 1, 1, 1 );
	gtk_widget_show( label );

	adj = gtk_adjustment_new( 0, 0, G_MAXINT, 1, 10, 0 );
	entity = spin = gtk_spin_button_new( GTK_ADJUSTMENT( adj ), 1, 0 );
	gtk_widget_set_hexpand( spin, TRUE );
	gtk_grid_attach( GTK_GRID( table ), spin, 1, 0, 1, 1 );
	gtk_spin_button_set_wrap( GTK_SPIN_BUTTON( spin ), FALSE );
	gtk_spin_button_set_numeric( GTK_SPIN_BUTTON( spin ), TRUE );
	gtk_widget_show( spin );
	g_object_set_data( G_OBJECT( dialog ), "entity-spin", spin );
	g_object_set( spin, "xalign", 1.0, NULL ); //right align numbers

	adj = gtk_adjustment_new( 0, 0, G_MAXINT, 1, 10, 0 );
	brush = spin = gtk_spin_button_new( GTK_ADJUSTMENT( adj ), 1, 0 );
	gtk_widget_set_hexpand( spin, TRUE );
	gtk_grid_attach( GTK_GRID( table ), spin, 1, 1, 1, 1 );
	gtk_spin_button_set_wrap( GTK_SPIN_BUTTON( spin ), FALSE );
	gtk_spin_button_set_numeric( GTK_SPIN_BUTTON( spin ), TRUE );
	gtk_widget_show( spin );
	g_object_set_data( G_OBJECT( dialog ), "brush-spin", spin );
	g_object_set( spin, "xalign", 1.0, NULL ); //right align numbers

	// Initialize dialog
	GetSelectionIndex( &ent, &br );

	gtk_spin_button_set_value( GTK_SPIN_BUTTON( entity ), ent );
	gtk_spin_button_set_value( GTK_SPIN_BUTTON( brush ), br );

	g_signal_connect( dialog, "response", G_CALLBACK( findbrushdialog_response ), dialog );

	gtk_widget_show( dialog );
}

// =============================================================================
// Arbitrary Sides dialog

void DoSides( bool bCone, bool bSphere, bool bTorus ){
	GtkWidget *dialog, *vbox, *hbox, *label, *content_area, *spin;
	GtkAdjustment *adj;
	gint response_id;
	GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;

	dialog = gtk_dialog_new_with_buttons( _( "Arbitrary sides" ), NULL, flags, NULL );
	gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "OK" ), GTK_RESPONSE_OK );
	gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "Cancel" ), GTK_RESPONSE_CANCEL );

	content_area = gtk_dialog_get_content_area( GTK_DIALOG( dialog ) );

	vbox = gtk_box_new( GTK_ORIENTATION_VERTICAL, 5 );
	gtk_container_add( GTK_CONTAINER( content_area ), vbox );
	gtk_container_set_border_width( GTK_CONTAINER( vbox ), 5 );
	gtk_widget_show( vbox );

	hbox = gtk_box_new( GTK_ORIENTATION_HORIZONTAL, 5 );
	gtk_box_pack_start( GTK_BOX( vbox ), hbox, TRUE, TRUE, 0 );
	gtk_container_set_border_width( GTK_CONTAINER( hbox ), 5 );
	gtk_widget_show( hbox );

	label = gtk_label_new( _( "Sides:" ) );
	gtk_box_pack_start( GTK_BOX( hbox ), label, FALSE, FALSE, 0 );
	gtk_widget_set_halign( label, GTK_ALIGN_START );
	gtk_widget_show( label );

	adj = gtk_adjustment_new( 3, 3, 100, 1, 10, 0 );
	spin = gtk_spin_button_new( adj, 1, 0 );
	gtk_box_pack_start( GTK_BOX( hbox ), spin, TRUE, TRUE, 0 );
	gtk_spin_button_set_wrap( GTK_SPIN_BUTTON( spin ), TRUE );
	gtk_spin_button_set_numeric( GTK_SPIN_BUTTON( spin ), TRUE );
	gtk_widget_set_hexpand( spin, TRUE );
	gtk_widget_show( spin );
	g_object_set( spin, "xalign", 1.0, NULL );

	gtk_widget_show( dialog );

	response_id = gtk_dialog_run( GTK_DIALOG( dialog ) );

	if( response_id == GTK_RESPONSE_OK ) 
	{
		int sides = gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON( spin ) );

		if ( bCone ) {
			Brush_MakeSidedCone( sides );
		}
		else if ( bSphere ) {
			Brush_MakeSidedSphere( sides );
		}
		else{
			Brush_MakeSided( sides );
		}
	}

	gtk_widget_destroy( dialog );
}

// =============================================================================
// New Patch dialog

void DoNewPatchDlg(){
	GtkWidget *dialog, *table, *vbox, *label, *combo;
	GtkWidget *width_combo, *height_combo, *content_area;
	GList *combo_list;
	GList *lst, *cells;
	gint response_id;
	GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;

	dialog = gtk_dialog_new_with_buttons( _( "Patch density" ), NULL, flags, NULL );
	gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "OK" ), GTK_RESPONSE_OK );
	gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "Cancel" ), GTK_RESPONSE_CANCEL );

	content_area = gtk_dialog_get_content_area( GTK_DIALOG( dialog ) );

	vbox = gtk_box_new( GTK_ORIENTATION_VERTICAL, 5 );
	gtk_container_add( GTK_CONTAINER( content_area ), vbox );
	gtk_container_set_border_width( GTK_CONTAINER( vbox ), 5 );
	gtk_widget_show( vbox );

	table = gtk_grid_new();
	gtk_box_pack_start( GTK_BOX( vbox ), table, TRUE, TRUE, 0 );
	gtk_grid_set_row_spacing( GTK_GRID( table ), 5 );
	gtk_grid_set_column_spacing( GTK_GRID( table ), 5 );
	gtk_widget_show( table );

	label = gtk_label_new( _( "Width:" ) );
	gtk_grid_attach( GTK_GRID( table ), label, 0, 0, 1, 1 );
	gtk_widget_set_halign( label, GTK_ALIGN_START );
	gtk_widget_show( label );

	label = gtk_label_new( _( "Height:" ) );
	gtk_grid_attach( GTK_GRID( table ), label, 0, 1, 1, 1 );
	gtk_widget_set_halign( label, GTK_ALIGN_START );
	gtk_widget_show( label );

	combo_list = (GList*)NULL;
	combo_list = g_list_append( combo_list, (void *)_( "3" ) );
	combo_list = g_list_append( combo_list, (void *)_( "5" ) );
	combo_list = g_list_append( combo_list, (void *)_( "7" ) );
	combo_list = g_list_append( combo_list, (void *)_( "9" ) );
	combo_list = g_list_append( combo_list, (void *)_( "11" ) );
	combo_list = g_list_append( combo_list, (void *)_( "13" ) );
	combo_list = g_list_append( combo_list, (void *)_( "15" ) );

	width_combo = combo = gtk_combo_box_text_new();
	for( lst = combo_list; lst != NULL; lst = g_list_next( lst ) )
	{
		gtk_combo_box_text_append( GTK_COMBO_BOX_TEXT( combo ), (const char *)lst->data, (const char *)lst->data );
	}
	gtk_combo_box_set_active( GTK_COMBO_BOX( combo ), 0 );
	gtk_grid_attach( GTK_GRID( table ), combo, 1, 0, 1, 1 );
	gtk_widget_set_hexpand( combo, TRUE );
	gtk_widget_show( combo );
//	g_object_set( combo, "xalign", 1.0, NULL );
	cells = gtk_cell_layout_get_cells( GTK_CELL_LAYOUT( combo ) );
	for( lst = cells; lst != NULL; lst = g_list_next( lst ) )
	{
		g_object_set( lst->data, "xalign", 1.0, NULL );
	}
	g_list_free( cells );

	height_combo = combo = gtk_combo_box_text_new();
	for( lst = combo_list; lst != NULL; lst = g_list_next( lst ) )
	{
		gtk_combo_box_text_append( GTK_COMBO_BOX_TEXT( combo ), (const char *)lst->data, (const char *)lst->data );
	}
	gtk_combo_box_set_active( GTK_COMBO_BOX( combo ), 0 );
	gtk_grid_attach( GTK_GRID( table ), combo, 1, 1, 1, 1 );
	gtk_widget_set_hexpand( combo, TRUE );
	gtk_widget_show( combo );
//	g_object_set( combo, "xalign", 1.0, NULL );
	cells = gtk_cell_layout_get_cells( GTK_CELL_LAYOUT( combo ) );
	for( lst = cells; lst != NULL; lst = g_list_next( lst ) )
	{
		g_object_set( lst->data, "xalign", 1.0, NULL );
	}
	g_list_free( cells );

	// Initialize dialog
	g_list_free( combo_list );

	gtk_widget_show( dialog );

	response_id = gtk_dialog_run( GTK_DIALOG( dialog ) );

	if( response_id == GTK_RESPONSE_OK ) 
	{
		const char* w = gtk_combo_box_text_get_active_text( GTK_COMBO_BOX_TEXT( width_combo ) );
		const char* h = gtk_combo_box_text_get_active_text( GTK_COMBO_BOX_TEXT( height_combo ) );

		Patch_GenericMesh( atoi( w ), atoi( h ), g_pParentWnd->ActiveXY()->GetViewType() );
		Sys_UpdateWindows( W_ALL );
	}

	gtk_widget_destroy( dialog );
}

// =============================================================================
// New Patch dialog

static void ScaleDialog_response( GtkWidget *widget, gint response_id, gpointer data ){
	GtkSpinButton *spin;
	float sx, sy, sz;

	if ( response_id == GTK_RESPONSE_CANCEL )
    {
		gtk_widget_destroy( GTK_WIDGET( widget ) );
		return;
	}
	if ( !( response_id == GTK_RESPONSE_OK || response_id == GTK_RESPONSE_APPLY ) )
    {
		return;
	}
	spin = GTK_SPIN_BUTTON( g_object_get_data( G_OBJECT( data ), "x" ) );
	sx = gtk_spin_button_get_value( spin );
	gtk_spin_button_set_value( GTK_SPIN_BUTTON( spin ), 1.0f );

	spin = GTK_SPIN_BUTTON( g_object_get_data( G_OBJECT( data ), "y" ) );
	sy = gtk_spin_button_get_value( spin );
	gtk_spin_button_set_value( GTK_SPIN_BUTTON( spin ), 1.0f );

	spin = GTK_SPIN_BUTTON( g_object_get_data( G_OBJECT( data ), "z" ) );
	sz = gtk_spin_button_get_value( spin );
	gtk_spin_button_set_value( GTK_SPIN_BUTTON( spin ), 1.0f );

	if ( sx > 0 && sy > 0 && sz > 0 ) {
		Select_Scale( sx, sy, sz );
		Sys_UpdateWindows( W_ALL );
	}
	else{
		Sys_Printf( "Warning.. Tried to scale by a zero value." );
	}

	if ( response_id == GTK_RESPONSE_OK )
    {
		gtk_widget_destroy( GTK_WIDGET( widget ) );
	}
}
void DoScaleDlg(){
	GtkWidget *dialog, *vbox1, *table, *label;
	GtkWidget *x, *y, *z, *content_area;
	GtkAdjustment *adj;
	GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;

	dialog = gtk_dialog_new_with_buttons( _( "Scale" ), NULL, flags, NULL );
	gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "OK" ), GTK_RESPONSE_OK );
	gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "Cancel" ), GTK_RESPONSE_CANCEL );
	gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "Apply" ), GTK_RESPONSE_APPLY );

	content_area = gtk_dialog_get_content_area( GTK_DIALOG( dialog ) );

	vbox1 = gtk_box_new( GTK_ORIENTATION_VERTICAL, 5 );
	gtk_container_add( GTK_CONTAINER( content_area ), vbox1 );
	gtk_container_set_border_width( GTK_CONTAINER( vbox1 ), 5 );
	gtk_widget_show( vbox1 );

	label = gtk_label_new( _( "Scale the selection around an axis with a factor:" ) );
	gtk_widget_set_halign( label, GTK_ALIGN_START );
	gtk_box_pack_start( GTK_BOX( vbox1 ), label, FALSE, FALSE, 0 );
	gtk_widget_show( label );

	table = gtk_grid_new();
	gtk_box_pack_start( GTK_BOX( vbox1 ), table, TRUE, TRUE, 0 );
	gtk_grid_set_row_spacing( GTK_GRID( table ), 5 );
	gtk_grid_set_column_spacing( GTK_GRID( table ), 5 );
	gtk_widget_show( table );

	label = gtk_label_new( _( "X:" ) );
	gtk_grid_attach( GTK_GRID( table ), label, 0, 0, 1, 1 );
	gtk_widget_set_halign( label, GTK_ALIGN_START );
	gtk_widget_show( label );

	adj = gtk_adjustment_new( 1.0, 1, 100, 0.1, 1, 0 );
	x = gtk_spin_button_new( GTK_ADJUSTMENT( adj ), 0.1, 1 );
	gtk_grid_attach( GTK_GRID( table ), x, 1, 0, 1, 1 );
	gtk_spin_button_set_wrap( GTK_SPIN_BUTTON( x ), TRUE );
	gtk_spin_button_set_numeric( GTK_SPIN_BUTTON( x ), TRUE );
	gtk_widget_set_hexpand( x, TRUE );
	gtk_widget_show( x );
	g_object_set_data( G_OBJECT( dialog ), "x", x );
	g_object_set( x, "xalign", 1.0, NULL ); //right align numbers

	label = gtk_label_new( _( "Y:" ) );
	gtk_grid_attach( GTK_GRID( table ), label, 0, 1, 1, 1 );
	gtk_widget_set_halign( label, GTK_ALIGN_START );
	gtk_widget_show( label );

	adj = gtk_adjustment_new( 1.0, 1, 100, 0.1, 1, 0 );
	y = gtk_spin_button_new( GTK_ADJUSTMENT( adj ), 0.1, 1 );
	gtk_grid_attach( GTK_GRID( table ), y, 1, 1, 1, 1 );
	gtk_spin_button_set_wrap( GTK_SPIN_BUTTON( y ), TRUE );
	gtk_spin_button_set_numeric( GTK_SPIN_BUTTON( y ), TRUE );
	gtk_widget_set_hexpand( y, TRUE );
	gtk_widget_show( y );
	g_object_set_data( G_OBJECT( dialog ), "y", y );
	g_object_set( y, "xalign", 1.0, NULL );

	label = gtk_label_new( _( "Z:" ) );
	gtk_grid_attach( GTK_GRID( table ), label, 0, 2, 1, 1 );
	gtk_widget_set_halign( label, GTK_ALIGN_START );
	gtk_widget_show( label );

	adj = gtk_adjustment_new( 1.0, 1, 100, 0.1, 1, 0 );
	z = gtk_spin_button_new( GTK_ADJUSTMENT( adj ), 0.1, 1 );
	gtk_grid_attach( GTK_GRID( table ), z, 1, 2, 1, 1 );
	gtk_spin_button_set_wrap( GTK_SPIN_BUTTON( z ), TRUE );
	gtk_spin_button_set_numeric( GTK_SPIN_BUTTON( z ), TRUE );
	gtk_widget_set_hexpand( z, TRUE );
	gtk_widget_show( z );
	g_object_set_data( G_OBJECT( dialog ), "z", z );
	g_object_set( z, "xalign", 1.0, NULL );

	g_signal_connect( dialog, "response", G_CALLBACK( ScaleDialog_response ), dialog );

	gtk_widget_show( dialog );
}

// =============================================================================
// Thicken Patch dialog

void DoThickenDlg(){
	GtkWidget *dialog, *vbox, *hbox, *label, *content_area;
	GtkWidget *amount, *seams, *group, *spin;
	GtkAdjustment *adj;
	gint response_id;
	GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;
	static qboolean bGroupResult = true;

	dialog = gtk_dialog_new_with_buttons( _( "Thicken Patch" ), NULL, flags, NULL );
	gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "OK" ), GTK_RESPONSE_OK );
	gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "Cancel" ), GTK_RESPONSE_CANCEL );

	content_area = gtk_dialog_get_content_area( GTK_DIALOG( dialog ) );

	vbox = gtk_box_new( GTK_ORIENTATION_VERTICAL, 5 );
	gtk_container_add( GTK_CONTAINER( content_area ), vbox );
	gtk_container_set_border_width( GTK_CONTAINER( vbox ), 5 );
	gtk_widget_show( vbox );

	label = gtk_label_new( _( "This produces a set of patches\n"
							  "that contains the original patch along with the\n"
							  "'thick' patch and an optimal set of seam patches." ) );
	gtk_box_pack_start( GTK_BOX( vbox ), label, TRUE, TRUE, 0 );
	gtk_widget_set_halign( label, GTK_ALIGN_START );
	gtk_widget_show( label );

	hbox = gtk_box_new( GTK_ORIENTATION_HORIZONTAL, 5 );
	gtk_box_pack_start( GTK_BOX( vbox ), hbox, FALSE, TRUE, 0 );
	gtk_widget_show( hbox );

	label = gtk_label_new( _( "Amount:" ) );
	gtk_box_pack_start( GTK_BOX( hbox ), label, FALSE, FALSE, 0 );
	gtk_widget_set_halign( label, GTK_ALIGN_START );
	gtk_widget_show( label );

	adj = gtk_adjustment_new( 1, 1, 100, 1, 10, 0 );
	amount = spin = gtk_spin_button_new( GTK_ADJUSTMENT( adj ), 1, 0 );
	gtk_box_pack_start( GTK_BOX( hbox ), spin, TRUE, TRUE, 0 );
	gtk_spin_button_set_wrap( GTK_SPIN_BUTTON( spin ), FALSE );
	gtk_spin_button_set_numeric( GTK_SPIN_BUTTON( spin ), TRUE );
	gtk_widget_set_hexpand( spin, TRUE );
	gtk_widget_show( spin );
	g_object_set( spin, "xalign", 1.0, NULL );

	seams = gtk_check_button_new_with_label( _( "Seams" ) );
	gtk_box_pack_start( GTK_BOX( vbox ), seams, TRUE, TRUE, 0 );
	gtk_widget_set_halign( seams, GTK_ALIGN_START );
	gtk_widget_show( seams );

	// bGroupResult
	group = gtk_check_button_new_with_label( _( "Result to func_group" ) );
	gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( group ), bGroupResult );
	gtk_box_pack_start( GTK_BOX( vbox ), group, FALSE, FALSE, 0 );
	gtk_widget_show( group );


	// Initialize dialog
	gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( seams ), TRUE );
	gtk_spin_button_set_value( GTK_SPIN_BUTTON( amount ), 8 );

	gtk_widget_show( dialog );

	response_id = gtk_dialog_run( GTK_DIALOG( dialog ) );

	if( response_id == GTK_RESPONSE_OK ) 
	{
		int new_amount;

		if ( gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( group ) ) ) {
			bGroupResult = true;
		}
		else{
			bGroupResult = false;
		}
		new_amount = gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON( amount ) );
		Patch_Thicken( new_amount, gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( seams ) ), bGroupResult );
		Sys_UpdateWindows( W_ALL );
	}

	gtk_widget_destroy( dialog );
}

// =============================================================================
// About dialog (no program is complete without one)

static const int ABT_WIDGET_PADDING = 8;

//! @note kaz 04/01/2012 - not in use
void about_button_changelog( GtkWidget *widget, gpointer data ){
	Str log;
	log = g_strAppPath;
	log += "changelog.txt";
	OpenURL( log.GetBuffer() );
}

//! @note kaz 04/01/2012 - not in use
void about_button_credits( GtkWidget *widget, gpointer data ){
	Str cred;
	cred = g_strAppPath;
	cred += "credits.html";
	OpenURL( cred.GetBuffer() );
}

void DoAbout(){
	GtkWidget *dialog, *content_area;
	gint response_id;
	GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;

	// create dialog window
	dialog = gtk_dialog_new_with_buttons( _( "About GtkRadiant" ), GTK_WINDOW( g_pParentWnd->m_pWidget ), flags, NULL );
	gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "OK" ), GTK_RESPONSE_OK );

	gtk_window_set_transient_for( GTK_WINDOW( dialog ), GTK_WINDOW( g_pParentWnd->m_pWidget ) );
	gtk_window_set_position( GTK_WINDOW( dialog ), GTK_WIN_POS_CENTER_ON_PARENT );
	gtk_window_set_resizable( GTK_WINDOW( dialog ), FALSE );  

	content_area = gtk_dialog_get_content_area( GTK_DIALOG( dialog ) );

	// layout top logo and everything else vertically without border padding
	GtkWidget *outer_vbox = gtk_box_new( GTK_ORIENTATION_VERTICAL, 0 );
	gtk_container_add( GTK_CONTAINER( content_area ), outer_vbox );
	gtk_container_set_border_width( GTK_CONTAINER( outer_vbox ), 0 ); 
	gtk_widget_show( outer_vbox );

	// radiant logo
	CString s = g_strBitmapsPath;
	s += "logo.png"; 
	GtkWidget *logo_image = gtk_image_new_from_file( s.GetBuffer() );
	gtk_box_pack_start( GTK_BOX( outer_vbox ), logo_image, FALSE, FALSE, 0 );
	gtk_widget_show( logo_image );

	// all other widgets layout
	GtkWidget *inner_vbox = gtk_box_new( GTK_ORIENTATION_VERTICAL, ABT_WIDGET_PADDING );
	gtk_box_pack_start( GTK_BOX( outer_vbox ), inner_vbox, FALSE, FALSE, 0 );
	gtk_container_set_border_width( GTK_CONTAINER( inner_vbox ), ABT_WIDGET_PADDING );
	gtk_widget_show( inner_vbox );

	// informative text
	GtkWidget *info_hbox = gtk_box_new( GTK_ORIENTATION_HORIZONTAL, 0 );
	gtk_box_pack_start( GTK_BOX( inner_vbox ), info_hbox, FALSE, FALSE, 0 );
	gtk_widget_show( info_hbox );

	GtkWidget *info_label = gtk_label_new( 
		"GtkRadiant " RADIANT_VERSION " - " __DATE__ "\n"
		RADIANT_ABOUTMSG "\n\n"
		"This product contains software technology from id Software, Inc.\n"
		"('id Technology'). id Technology 2000 id Software, Inc.\n\n"
		"Visit http://icculus.org/gtkradiant/ to view a full list of credits,\n"
		"changelogs, and to report problems with this software." );
	gtk_box_pack_start( GTK_BOX( info_hbox ), info_label, FALSE, FALSE, 0 );
	gtk_widget_set_halign( info_label, GTK_ALIGN_START );
	gtk_widget_show( info_label );

	// OpenGL properties 
	GtkWidget *gl_prop_frame = gtk_frame_new( _( "OpenGL Properties" ) );
	gtk_box_pack_start( GTK_BOX( inner_vbox ), gl_prop_frame, TRUE, TRUE, 0 );
	gtk_widget_set_hexpand( gl_prop_frame, TRUE );
	gtk_widget_show( gl_prop_frame );

	GtkWidget *gl_prop_table = gtk_grid_new();
	gtk_container_add( GTK_CONTAINER( gl_prop_frame ), gl_prop_table );
	gtk_container_set_border_width( GTK_CONTAINER( gl_prop_table ), 4 );
	gtk_grid_set_row_spacing( GTK_GRID( gl_prop_table ), 4 );
	gtk_grid_set_column_spacing( GTK_GRID( gl_prop_table ), 4 );
	gtk_widget_set_hexpand( gl_prop_table, TRUE );
	gtk_widget_show( gl_prop_table );

	GtkWidget *vendor_label = gtk_label_new( _( "Vendor:" ) );
	gtk_grid_attach( GTK_GRID( gl_prop_table ), vendor_label, 0, 0, 1, 1 );
	gtk_widget_set_halign( vendor_label, GTK_ALIGN_START );
	gtk_widget_show( vendor_label );

	GtkWidget *version_label = gtk_label_new( _( "Version:" ) );
	gtk_grid_attach( GTK_GRID( gl_prop_table ), version_label, 0, 1, 1, 1 );
	gtk_widget_set_halign( version_label, GTK_ALIGN_START );
	gtk_widget_show( version_label );

	GtkWidget *renderer_label = gtk_label_new( _( "Renderer:" ) );
	gtk_grid_attach( GTK_GRID( gl_prop_table ), renderer_label, 0, 2, 1, 1 );
	gtk_widget_set_halign( renderer_label, GTK_ALIGN_START );
	gtk_widget_show( renderer_label );

	GtkWidget *gl_vendor_label = gtk_label_new( (char*)qglGetString( GL_VENDOR ) );
	gtk_grid_attach( GTK_GRID( gl_prop_table ), gl_vendor_label, 1, 0, 1, 1 );
	gtk_widget_set_hexpand( gl_vendor_label, TRUE );
	gtk_widget_set_halign( gl_vendor_label, GTK_ALIGN_START );
	gtk_widget_show( gl_vendor_label );

	GtkWidget *gl_version_label = gtk_label_new( (char*)qglGetString( GL_VERSION ) );
	gtk_grid_attach( GTK_GRID( gl_prop_table ), gl_version_label, 1, 1, 1, 1 );
	gtk_widget_set_hexpand( gl_version_label, TRUE );
	gtk_widget_set_halign( gl_version_label, GTK_ALIGN_START );
	gtk_widget_show( gl_version_label );

	GtkWidget *gl_renderer_label = gtk_label_new( (char*)qglGetString( GL_RENDERER ) );
	gtk_grid_attach( GTK_GRID( gl_prop_table ), gl_renderer_label, 1, 2, 1, 1 );
	gtk_widget_set_hexpand( gl_renderer_label, TRUE );
	gtk_widget_set_halign( gl_renderer_label, GTK_ALIGN_START );
	gtk_widget_show( gl_renderer_label );

	// OpenGL extensions
	GtkWidget *gl_ext_frame = gtk_frame_new( _( "OpenGL Extensions" ) );
	gtk_box_pack_start( GTK_BOX( inner_vbox ), gl_ext_frame, TRUE, TRUE, 0 );
	gtk_widget_set_hexpand( gl_ext_frame, TRUE );
	gtk_widget_show( gl_ext_frame );

	GtkWidget *gl_ext_hbox = gtk_box_new( GTK_ORIENTATION_HORIZONTAL, ABT_WIDGET_PADDING );
	gtk_container_add( GTK_CONTAINER( gl_ext_frame ), gl_ext_hbox );
	gtk_container_set_border_width( GTK_CONTAINER( gl_ext_hbox ), 4 );	
	gtk_widget_show( gl_ext_hbox );

	GtkWidget *gl_ext_scroll = gtk_scrolled_window_new( NULL, NULL );
	gtk_box_pack_start( GTK_BOX( gl_ext_hbox ), gl_ext_scroll, TRUE, TRUE, 0 );
	gtk_scrolled_window_set_policy( GTK_SCROLLED_WINDOW( gl_ext_scroll ), GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS );
	gtk_scrolled_window_set_shadow_type( GTK_SCROLLED_WINDOW( gl_ext_scroll ), GTK_SHADOW_IN );
	gtk_widget_show( gl_ext_scroll );

	GtkWidget *gl_ext_textview = gtk_text_view_new();
	gtk_text_view_set_editable( GTK_TEXT_VIEW( gl_ext_textview ), FALSE );
	gtk_container_add( GTK_CONTAINER( gl_ext_scroll ), gl_ext_textview );
	GtkTextBuffer* buffer = gtk_text_view_get_buffer( GTK_TEXT_VIEW( gl_ext_textview ) );
	gtk_text_buffer_set_text( buffer, (char *)qglGetString( GL_EXTENSIONS ), -1 );
	gtk_text_view_set_wrap_mode( GTK_TEXT_VIEW( gl_ext_textview ), GTK_WRAP_WORD );;
	gtk_widget_show( gl_ext_textview );

	// buttons
	/*
	button = gtk_button_new_with_label( _( "Credits" ) );
	gtk_widget_show( button );
	gtk_box_pack_end( GTK_BOX( button_hbox ), button, FALSE, FALSE, 0 );
	g_signal_connect( button, "clicked",
						G_CALLBACK( about_button_credits ), NULL );

	button = gtk_button_new_with_label( _( "Changelog" ) );
	gtk_widget_show( button );
	gtk_box_pack_end( GTK_BOX( button_hbox ), button, FALSE, FALSE, 0 );
	g_signal_connect( button, "clicked",
						G_CALLBACK( about_button_changelog ), NULL );
	*/

	// show it
	gtk_widget_show( dialog );

	response_id = gtk_dialog_run( GTK_DIALOG( dialog ) );

	gtk_widget_destroy( dialog );
}

// =============================================================================
// Command List dialog

void DoCommandListDlg(){
	GtkWidget *dialog, *vbox1, *scr, *content_area;
	gint response_id;
	GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;

	dialog = gtk_dialog_new_with_buttons( _( "Shortcut List" ), GTK_WINDOW( g_pParentWnd->m_pWidget ), flags, NULL );
	gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "OK" ), GTK_RESPONSE_OK );

    gtk_window_set_transient_for( GTK_WINDOW( dialog ), GTK_WINDOW( g_pParentWnd->m_pWidget ) );
	gtk_window_set_position( GTK_WINDOW( dialog ), GTK_WIN_POS_CENTER_ON_PARENT );
	gtk_window_set_default_size( GTK_WINDOW( dialog ), 400, 400 );

	content_area = gtk_dialog_get_content_area( GTK_DIALOG( dialog ) );

	vbox1 = gtk_box_new( GTK_ORIENTATION_VERTICAL, 5 );
	gtk_container_add( GTK_CONTAINER( content_area ), vbox1 );
	gtk_container_set_border_width( GTK_CONTAINER( vbox1 ), 5 );
	gtk_widget_show( vbox1 );

	scr = gtk_scrolled_window_new( (GtkAdjustment*)NULL, (GtkAdjustment*)NULL );
	gtk_box_pack_start( GTK_BOX( vbox1 ), scr, TRUE, TRUE, 0 );
	gtk_scrolled_window_set_policy( GTK_SCROLLED_WINDOW( scr ), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC );
	gtk_scrolled_window_set_shadow_type( GTK_SCROLLED_WINDOW( scr ), GTK_SHADOW_IN );
	gtk_widget_set_hexpand( scr, TRUE );
	gtk_widget_set_vexpand( scr, TRUE );
	gtk_widget_show( scr );

	{
		GtkListStore* store = gtk_list_store_new( 2, G_TYPE_STRING, G_TYPE_STRING );

		GtkWidget* view = gtk_tree_view_new_with_model( GTK_TREE_MODEL( store ) );
//		gtk_tree_selection_set_mode( gtk_tree_view_get_selection( GTK_TREE_VIEW( view ) ), GTK_SELECTION_NONE );
		{
			GtkCellRenderer* renderer = gtk_cell_renderer_text_new();
			GtkTreeViewColumn* column = gtk_tree_view_column_new_with_attributes( _( "Command" ), renderer, "text", 0, (char *) NULL );
			gtk_tree_view_append_column( GTK_TREE_VIEW( view ), column );
		}

		{
			GtkCellRenderer* renderer = gtk_cell_renderer_text_new();
			GtkTreeViewColumn* column = gtk_tree_view_column_new_with_attributes( _( "Key" ), renderer, "text", 1, (char *) NULL );
			gtk_tree_view_append_column( GTK_TREE_VIEW( view ), column );
		}

		gtk_container_add( GTK_CONTAINER( scr ), view );
		gtk_widget_show( view );

		{
			// Initialize dialog
			CString path;
			path = g_strTempPath;
			path += "commandlist.txt";

			GSList *cmds = NULL;
			int n;

			for ( n = 0; n < g_nCommandCount; n++ )
				cmds = g_slist_append( cmds, (gpointer)g_Commands[n].m_strCommand );
			cmds = g_slist_sort( cmds, ( gint ( * )( const void *, const void * ) )strcmp );

			Sys_Printf( "Writing the command list to %s", path.GetBuffer() );
			FILE * fileout = fopen( path.GetBuffer(), "wt" );

			while ( cmds )
			{
				for ( n = 0; n < g_nCommandCount; n++ )
					if ( cmds->data == g_Commands[n].m_strCommand ) {
						break;
					}

				char c = g_Commands[n].m_nKey;
				CString strLine, strMod( "" ), strKeys( c );

				for ( int k = 0; k < g_nKeyCount; k++ )
				{
					if ( g_Keys[k].m_nVKKey == g_Commands[n].m_nKey ) {
						strKeys = g_Keys[k].m_strName;
						break;
					}
				}

				if ( g_Commands[n].m_nModifiers & RAD_SHIFT ) {
					strMod = "Shift";
				}
				if ( g_Commands[n].m_nModifiers & RAD_ALT ) {
					strMod += ( strMod.GetLength() > 0 ) ? " + Alt" : "Alt";
				}
				if ( g_Commands[n].m_nModifiers & RAD_CONTROL ) {
					strMod += ( strMod.GetLength() > 0 ) ? " + Control" : "Control";
				}
				if ( strMod.GetLength() > 0 ) {
					strMod += " + ";
				}
				strMod += strKeys;

				{
					GtkTreeIter iter;
					gtk_list_store_append( store, &iter );
					gtk_list_store_set( store, &iter, 0, g_Commands[n].m_strCommand, 1, strMod.GetBuffer(), -1 );
				}

				if ( fileout != NULL ) {
					strLine.Format( "%-25s %s\r\n", g_Commands[n].m_strCommand, strMod.GetBuffer() );
					fputs( strLine.GetBuffer(), fileout );
				}

				cmds = g_slist_remove( cmds, cmds->data );
			}

			if ( fileout != NULL ) {
				fclose( fileout );
			}
		}

		g_object_unref( G_OBJECT( store ) );
	}


	gtk_widget_show( dialog );

	response_id = gtk_dialog_run( GTK_DIALOG( dialog ) );

	gtk_widget_destroy( dialog );
}

// =============================================================================
// Texture List dialog

static void TextureListDialog_response( GtkWidget *widget, gint response_id, gpointer data ){
	GtkTreeSelection* selection;

	GtkTreeModel* model;
	GtkTreeIter iter;

	if ( response_id == GTK_RESPONSE_CANCEL )
    {
		gtk_widget_destroy( GTK_WIDGET( widget ) );
		return;
	}
	if ( !( response_id == GTK_RESPONSE_OK || response_id == GTK_RESPONSE_APPLY ) )
    {
		return;
	}

	selection = gtk_tree_view_get_selection( GTK_TREE_VIEW( data ) );

	if ( gtk_tree_selection_get_selected( selection, &model, &iter ) ) {
		GtkTreePath* path = gtk_tree_model_get_path( model, &iter );
		if ( gtk_tree_path_get_depth( path ) == 1 ) {
			char* p;
			gtk_tree_model_get( model, &iter, 0, &p, -1 );
			
			Texture_ShowDirectory_by_path( p );
			g_free( p );
		}
		gtk_tree_path_free( path );
	}

	if ( response_id == GTK_RESPONSE_OK )
    {
		gtk_widget_destroy( GTK_WIDGET( widget ) );
	}
}
void DoTextureListDlg(){
	GtkWidget *dialog, *hbox, *scr, *content_area;
	GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;

	dialog = gtk_dialog_new_with_buttons( _( "Textures" ), NULL, flags, NULL );
	gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "Load" ), GTK_RESPONSE_OK );
	gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "Cancel" ), GTK_RESPONSE_CANCEL );

	gtk_window_set_default_size( GTK_WINDOW( dialog ), 400, 400 );

	content_area = gtk_dialog_get_content_area( GTK_DIALOG( dialog ) );

	hbox = gtk_box_new( GTK_ORIENTATION_HORIZONTAL, 5 );
	gtk_container_add( GTK_CONTAINER( content_area ), hbox );
	gtk_container_set_border_width( GTK_CONTAINER( hbox ), 5 );
	gtk_widget_show( hbox );

	scr = gtk_scrolled_window_new( (GtkAdjustment*)NULL, (GtkAdjustment*)NULL );
	gtk_box_pack_start( GTK_BOX( hbox ), scr, TRUE, TRUE, 0 );
	gtk_scrolled_window_set_policy( GTK_SCROLLED_WINDOW( scr ), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC );
	gtk_scrolled_window_set_shadow_type( GTK_SCROLLED_WINDOW( scr ), GTK_SHADOW_IN );
	gtk_widget_show( scr );

	GtkWidget* texture_list;

	{
		GtkListStore* store = gtk_list_store_new( 1, G_TYPE_STRING );

		GtkWidget* view = gtk_tree_view_new_with_model( GTK_TREE_MODEL( store ) );
		gtk_widget_set_vexpand( view, TRUE );
		gtk_widget_set_hexpand( view, TRUE );
		gtk_tree_view_set_headers_visible( GTK_TREE_VIEW( view ), FALSE );

		{
			GtkCellRenderer* renderer = gtk_cell_renderer_text_new();
			GtkTreeViewColumn* column = gtk_tree_view_column_new_with_attributes( "Textures", renderer, "text", 0, (char *) NULL );
			gtk_tree_view_append_column( GTK_TREE_VIEW( view ), column );
		}

		gtk_container_add( GTK_CONTAINER( scr ), view );

		{
			// Initialize dialog
			GtkTreeIter iter;
			GSList *textures = (GSList*)NULL;
			FillTextureList( &textures );
			while ( textures != NULL )
			{
				gtk_list_store_append( store, &iter );
				gtk_list_store_set( store, &iter, 0, (gchar*)textures->data, -1 );

				g_free( textures->data );
				textures = g_slist_remove( textures, textures->data );
			}
		}

		g_object_unref( G_OBJECT( store ) );
		gtk_widget_show( view );

		texture_list = view;
	}

	g_signal_connect( dialog, "response", G_CALLBACK( TextureListDialog_response ), texture_list );

	gtk_widget_show( dialog );
}

// =============================================================================
// Cap dialog

int DoCapDlg( int *type, bool *b_GroupResult ){
	GtkWidget *dialog, *vbox, *table, *pixmap, *group_toggle, *content_area;
	GtkWidget *bevel, *endcap, *ibevel, *iendcap;
	gint response_id;
	int ret;
	GSList *group = (GSList*)NULL;
	GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;

	dialog = gtk_dialog_new_with_buttons( _( "Cap" ), NULL, flags, NULL );
	gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "OK" ), GTK_RESPONSE_OK );
	gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "Cancel" ), GTK_RESPONSE_CANCEL );

	content_area = gtk_dialog_get_content_area( GTK_DIALOG( dialog ) );

	vbox = gtk_box_new( GTK_ORIENTATION_VERTICAL, 5 );
	gtk_box_pack_start( GTK_BOX( content_area ), vbox, FALSE, FALSE, 0 );
	gtk_widget_show( vbox );

	table = gtk_grid_new();
	gtk_box_pack_start( GTK_BOX( vbox ), table, TRUE, TRUE, 0 );
	gtk_grid_set_row_spacing( GTK_GRID( table ), 5 );
	gtk_grid_set_column_spacing( GTK_GRID( table ), 5 );
	gtk_widget_show( table );

	pixmap = new_image_icon( "cap_bevel.png" );
	gtk_grid_attach( GTK_GRID( table ), pixmap, 0, 0, 1, 1 );
	gtk_widget_show( pixmap );

	pixmap = new_image_icon( "cap_endcap.png" );
	gtk_grid_attach( GTK_GRID( table ), pixmap, 0, 1, 1, 1 );
	gtk_widget_show( pixmap );

	pixmap = new_image_icon( "cap_ibevel.png" );
	gtk_grid_attach( GTK_GRID( table ), pixmap, 0, 2, 1, 1 );
	gtk_widget_show( pixmap );

	pixmap = new_image_icon( "cap_iendcap.png" );
	gtk_grid_attach( GTK_GRID( table ), pixmap, 0, 3, 1, 1 );
	gtk_widget_show( pixmap );

	bevel = gtk_radio_button_new_with_label( group, _( "Bevel" ) );
	gtk_grid_attach( GTK_GRID( table ), bevel, 1, 0, 1, 1 );
	gtk_widget_show( bevel );

	group = gtk_radio_button_get_group( GTK_RADIO_BUTTON( bevel ) );

	endcap = gtk_radio_button_new_with_label( group, _( "Endcap" ) );
	gtk_grid_attach( GTK_GRID( table ), endcap, 1, 1, 1, 1 );
	group = gtk_radio_button_get_group( GTK_RADIO_BUTTON( endcap ) );
	gtk_widget_show( endcap );

	ibevel = gtk_radio_button_new_with_label( group, _( "Inverted Bevel" ) );
	gtk_grid_attach( GTK_GRID( table ), ibevel, 1, 2, 1, 1 );
	group = gtk_radio_button_get_group( GTK_RADIO_BUTTON( ibevel ) );
	gtk_widget_show( ibevel );

	iendcap = gtk_radio_button_new_with_label( group, _( "Inverted Endcap" ) );
	gtk_grid_attach( GTK_GRID( table ), iendcap, 1, 3, 1, 1 );
	group = gtk_radio_button_get_group( GTK_RADIO_BUTTON( iendcap ) );
	gtk_widget_show( iendcap );

	// Gef: added radio toggle for func_grouping capped patches
	group_toggle = gtk_check_button_new_with_label( _( "Result to func_group" ) );
	gtk_container_add( GTK_CONTAINER( vbox ), group_toggle );
	gtk_widget_show( group_toggle );

	// Gef: Set the state of the func_group toggle
	gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( group_toggle ), *b_GroupResult );

	// Initialize dialog
	gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( bevel ), TRUE );

	gtk_widget_show( dialog );

	response_id = gtk_dialog_run( GTK_DIALOG( dialog ) );

	if( response_id == GTK_RESPONSE_OK ) 
	{
		if ( gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( bevel ) ) ) {
			*type = BEVEL; //*type = CapDialog::BEVEL;
		}
		else if ( gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( endcap ) ) ) {
			*type = ENDCAP; //*type = CapDialog::ENDCAP;
		}
		else if ( gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( ibevel ) ) ) {
			*type = IBEVEL; // *type = CapDialog::IBEVEL;
		}
		else if ( gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( iendcap ) ) ) {
			*type = IENDCAP; // *type = CapDialog::IENDCAP;

		}
		// Gef: Added toggle for optional cap func_grouping
		*b_GroupResult = (bool)gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( group_toggle ) );

		ret = IDOK;
	} else {
		ret = IDCANCEL;
	}

	gtk_widget_destroy( dialog );

	return ret;
}

// =============================================================================
// Scripts dialog

void DoScriptsDlg(){
	GtkWidget *dialog, *vbox, *vbox2, *hbox, *label, *button, *scr;
	GtkWidget *run_button, *new_button, *edit_button, *content_area;
	GtkSizeGroup *button_group;
	gint response_id;
	GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;

	dialog = gtk_dialog_new_with_buttons( _( "Available Scripts - Not Implemented Yet" ), NULL, flags, NULL );
//	gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "OK" ), GTK_RESPONSE_OK );
//	gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "Cancel" ), GTK_RESPONSE_CANCEL );
	gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "Close" ), GTK_RESPONSE_CANCEL );
	
	content_area = gtk_dialog_get_content_area( GTK_DIALOG( dialog ) );

	vbox = gtk_box_new( GTK_ORIENTATION_VERTICAL, 5 );
	gtk_container_add( GTK_CONTAINER( content_area ), vbox );
	gtk_container_set_border_width( GTK_CONTAINER( vbox ), 5 );
	gtk_widget_show( vbox );

	label = gtk_label_new( _( "WARNING: BrushScripting is in a highly experimental state and is\n"
							  "far from complete. If you attempt to use them it is VERY LIKELY\n"
							  "that Radiant will crash. Save your work before attempting to\n"
							  "make use of any scripting features." ) );
	gtk_box_pack_start( GTK_BOX( vbox ), label, FALSE, FALSE, 0 );
	gtk_widget_set_halign( label, GTK_ALIGN_START );
	gtk_widget_show( label );

	hbox = gtk_box_new( GTK_ORIENTATION_HORIZONTAL, 5 );
	gtk_box_pack_start( GTK_BOX( vbox ), hbox, TRUE, TRUE, 0 );
	gtk_widget_show( hbox );

	scr = gtk_scrolled_window_new( (GtkAdjustment*)NULL, (GtkAdjustment*)NULL );
	gtk_box_pack_start( GTK_BOX( hbox ), scr, TRUE, TRUE, 0 );
	gtk_scrolled_window_set_policy( GTK_SCROLLED_WINDOW( scr ), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC );
	gtk_scrolled_window_set_shadow_type( GTK_SCROLLED_WINDOW( scr ), GTK_SHADOW_IN );
	gtk_widget_set_hexpand( scr, TRUE );
	gtk_widget_set_vexpand( scr, TRUE );
	gtk_widget_show( scr );

	GtkWidget* scripts_list;

	{
		GtkListStore* store = gtk_list_store_new( 1, G_TYPE_STRING );

		GtkWidget* view = gtk_tree_view_new_with_model( GTK_TREE_MODEL( store ) );
		gtk_tree_view_set_headers_visible( GTK_TREE_VIEW( view ), FALSE );

		{
			GtkCellRenderer* renderer = gtk_cell_renderer_text_new();
			GtkTreeViewColumn* column = gtk_tree_view_column_new_with_attributes( "", renderer, "text", 0, (char *) NULL );
			gtk_tree_view_append_column( GTK_TREE_VIEW( view ), column );
		}

		gtk_widget_show( view );
		gtk_container_add( GTK_CONTAINER( scr ), view );

		{
			// Initialize dialog
			CString strINI;
			strINI = g_strGameToolsPath;
			strINI += "/scripts.ini";
			FILE *f;

			f = fopen( strINI.GetBuffer(), "rt" );
			if ( f != NULL ) {
				char line[1024], *ptr;

				// read section names
				while ( fgets( line, 1024, f ) != 0 )
				{
					if ( line[0] != '[' ) {
						continue;
					}

					ptr = strchr( line, ']' );
					*ptr = '\0';

					{
						GtkTreeIter iter;
						gtk_list_store_append( store, &iter );
						gtk_list_store_set( store, &iter, 0, line, -1 );
					}
				}
				fclose( f );
			}
		}

		g_object_unref( G_OBJECT( store ) );

		scripts_list = view;
	}

	vbox2 = gtk_box_new( GTK_ORIENTATION_VERTICAL, 5 );
	gtk_box_pack_start( GTK_BOX( hbox ), vbox2, FALSE, FALSE, 0 );
	gtk_widget_show( vbox2 );

	run_button = button = gtk_button_new_with_label( _( "Run" ) );
	gtk_box_pack_start( GTK_BOX( vbox2 ), button, FALSE, FALSE, 0 );
	g_signal_connect( button, "clicked",
						G_CALLBACK( dialog_button_callback ), GINT_TO_POINTER( IDOK ) );
	gtk_widget_show( button );

	new_button = button = gtk_button_new_with_label( _( "New..." ) );
	gtk_box_pack_start( GTK_BOX( vbox2 ), button, FALSE, FALSE, 0 );
	gtk_widget_set_sensitive( button, FALSE );
	gtk_widget_show( button );

	edit_button = button = gtk_button_new_with_label( _( "Edit..." ) );
	gtk_box_pack_start( GTK_BOX( vbox2 ), button, FALSE, FALSE, 0 );
	gtk_widget_set_sensitive( button, FALSE );
	gtk_widget_show( button );

	button_group = gtk_size_group_new( GTK_SIZE_GROUP_BOTH );
	gtk_size_group_add_widget( button_group, run_button );
	gtk_size_group_add_widget( button_group, new_button );
	gtk_size_group_add_widget( button_group, edit_button );
	g_object_unref( button_group );

	gtk_widget_show( dialog );

	response_id = gtk_dialog_run( GTK_DIALOG( dialog ) );

	if( response_id == GTK_RESPONSE_OK ) 
	{
		GtkTreeSelection* selection = gtk_tree_view_get_selection( GTK_TREE_VIEW( scripts_list ) );

		GtkTreeModel* model;
		GtkTreeIter iter;
		if ( gtk_tree_selection_get_selected( selection, &model, &iter ) ) {
			char* script;
			gtk_tree_model_get( model, &iter, 0, &script, -1 );
			RunScriptByName( script, true );
			g_free( script );
		}
	}

	gtk_widget_destroy( dialog );
}

// =============================================================================
//  dialog

int DoBSInputDlg( const char *fields[5], float values[5] ){
	GtkWidget *dialog, *vbox, *label, *content_area;
	GtkWidget *entries[5];
	int i, ret;
	gint response_id;
	GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;

	dialog = gtk_dialog_new_with_buttons( _( "BrushScript Input" ), NULL, flags, NULL );
	gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "OK" ), GTK_RESPONSE_OK );
	gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "Cancel" ), GTK_RESPONSE_CANCEL );

	content_area = gtk_dialog_get_content_area( GTK_DIALOG( dialog ) );

	vbox = gtk_box_new( GTK_ORIENTATION_VERTICAL, 5 );
	gtk_box_pack_start( GTK_BOX( content_area ), vbox, TRUE, TRUE, 0 );
	gtk_widget_show( vbox );

	// Create entries and initialize them
	for ( i = 0; i < 5; i++ )
	{
		if ( strlen( fields[i] ) == 0 ) {
			continue;
		}

		label = gtk_label_new( fields[i] );
		gtk_box_pack_start( GTK_BOX( vbox ), label, FALSE, FALSE, 0 );
		gtk_widget_set_halign( label, GTK_ALIGN_START );
		gtk_widget_show( label );

		entries[i] = gtk_entry_new();
		gtk_box_pack_start( GTK_BOX( vbox ), entries[i], TRUE, TRUE, 0 );
		gtk_widget_show( entries[i] );

		char buf[64];
		snprintf( buf, sizeof( buf ), "%f", values[i] );
		gtk_entry_set_text( GTK_ENTRY( entries[i] ), buf );
	}

	gtk_widget_show( dialog );

	response_id = gtk_dialog_run( GTK_DIALOG( dialog ) );

	for ( i = 0; i < 5; i++ )
	{
		if ( strlen( fields[i] ) == 0 ) {
			continue;
		}

		values[i] = atof( gtk_entry_get_text( GTK_ENTRY( entries[i] ) ) );
	}
	switch( response_id )
	{
	case GTK_RESPONSE_OK:
		ret = IDOK;
		break;
	//case GTK_RESPONSE_CANCEL:
	default:
		ret = IDCANCEL;
		break;
	}

	gtk_widget_destroy( dialog );

	return ret;
}

// =============================================================================
// TextureLayout dialog

int DoTextureLayout( float *fx, float *fy ){
	GtkWidget *dialog, *vbox, *hbox, *table, *label;
	GtkWidget *x, *y, *content_area;
	gint response_id;
	int ret;
	GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;

	dialog = gtk_dialog_new_with_buttons( _( "Patch texture layout" ), NULL, flags, NULL );
	gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "OK" ), GTK_RESPONSE_OK );
	gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "Cancel" ), GTK_RESPONSE_CANCEL );

	content_area = gtk_dialog_get_content_area( GTK_DIALOG( dialog ) );

	hbox = gtk_box_new( GTK_ORIENTATION_HORIZONTAL, 5 );
	gtk_container_add( GTK_CONTAINER( content_area ), hbox );
	gtk_container_set_border_width( GTK_CONTAINER( hbox ), 5 );
	gtk_widget_show( hbox );

	vbox = gtk_box_new( GTK_ORIENTATION_VERTICAL, 5 );
	gtk_box_pack_start( GTK_BOX( hbox ), vbox, TRUE, TRUE, 0 );
	gtk_widget_show( vbox );

	label = gtk_label_new( _( "Texture will be fit across the patch based\n"
							  "on the x and y values given. Values of 1x1\n"
							  "will \"fit\" the texture. 2x2 will repeat\n"
							  "it twice, etc." ) );
	gtk_box_pack_start( GTK_BOX( vbox ), label, TRUE, TRUE, 0 );
	gtk_widget_set_halign( label, GTK_ALIGN_START );
	gtk_widget_show( label );

	table = gtk_grid_new();
	gtk_box_pack_start( GTK_BOX( vbox ), table, TRUE, TRUE, 0 );
	gtk_grid_set_row_spacing( GTK_GRID( table ), 5 );
	gtk_grid_set_column_spacing( GTK_GRID( table ), 5 );
	gtk_widget_show( table );

	label = gtk_label_new( _( "Texture x:" ) );
	gtk_grid_attach( GTK_GRID( table ), label, 0, 0, 1, 1 );
	gtk_widget_set_halign( label, GTK_ALIGN_START );
	gtk_widget_show( label );

	label = gtk_label_new( _( "Texture y:" ) );
	gtk_grid_attach( GTK_GRID( table ), label, 0, 1, 1, 1 );
	gtk_widget_set_halign( label, GTK_ALIGN_START );
	gtk_widget_show( label );

	x = gtk_entry_new();
	gtk_grid_attach( GTK_GRID( table ), label, 1, 0, 1, 1 );
	gtk_widget_show( x );

	y = gtk_entry_new();
	gtk_grid_attach( GTK_GRID( table ), label, 1, 1, 1, 1 );
	gtk_widget_show( y );


	// Initialize
	gtk_entry_set_text( GTK_ENTRY( x ), _( "4.0" ) );
	gtk_entry_set_text( GTK_ENTRY( y ), _( "4.0" ) );

	gtk_widget_show( dialog );

	response_id = gtk_dialog_run( GTK_DIALOG( dialog ) );

	if( response_id == GTK_RESPONSE_OK ) 
	{
		*fx = atof( gtk_entry_get_text( GTK_ENTRY( x ) ) );
		*fy = atof( gtk_entry_get_text( GTK_ENTRY( y ) ) );

		ret = IDOK;
	} else {
		ret = IDCANCEL;
	}

	gtk_widget_destroy( dialog );

	return ret;
}

// =============================================================================
// Name dialog

char* DoNameDlg( const char* title ){
	GtkWidget *dlg, *vbox, *hbox, *label, *button, *entry;
	int loop = 1, ret = IDCANCEL;
	char *str;

	dlg = gtk_window_new( GTK_WINDOW_TOPLEVEL );
	gtk_window_set_title( GTK_WINDOW( dlg ), title );
	g_signal_connect( dlg, "delete_event",
						G_CALLBACK( dialog_delete_callback ), NULL );
	g_signal_connect( dlg, "destroy",
						G_CALLBACK( gtk_widget_destroy ), NULL );
	g_object_set_data( G_OBJECT( dlg ), "loop", &loop );
	g_object_set_data( G_OBJECT( dlg ), "ret", &ret );

	hbox = gtk_box_new( GTK_ORIENTATION_HORIZONTAL, 5 );
	gtk_widget_show( hbox );
	gtk_container_add( GTK_CONTAINER( dlg ), hbox );
	gtk_container_set_border_width( GTK_CONTAINER( hbox ), 5 );

	label = gtk_label_new( _( "Name:" ) );
	gtk_widget_show( label );
	gtk_box_pack_start( GTK_BOX( hbox ), label, FALSE, FALSE, 0 );

	entry = gtk_entry_new();
	gtk_widget_show( entry );
	gtk_box_pack_start( GTK_BOX( hbox ), entry, TRUE, TRUE, 0 );

	vbox = gtk_box_new( GTK_ORIENTATION_VERTICAL, 5 );
	gtk_widget_show( vbox );
	gtk_box_pack_start( GTK_BOX( hbox ), vbox, FALSE, FALSE, 0 );

	button = gtk_button_new_with_label( _( "OK" ) );
	gtk_widget_show( button );
	gtk_box_pack_start( GTK_BOX( vbox ), button, FALSE, FALSE, 0 );
	g_signal_connect( button, "clicked",
						G_CALLBACK( dialog_button_callback ), GINT_TO_POINTER( IDOK ) );
	gtk_widget_set_size_request( button, 60, -2 );

	button = gtk_button_new_with_label( _( "Cancel" ) );
	gtk_widget_show( button );
	gtk_box_pack_start( GTK_BOX( vbox ), button, FALSE, FALSE, 0 );
	g_signal_connect( button, "clicked",
						G_CALLBACK( dialog_button_callback ), GINT_TO_POINTER( IDCANCEL ) );
	gtk_widget_set_size_request( button, 60, -2 );

	gtk_grab_add( dlg );
	gtk_widget_show( dlg );

	while ( loop )
		gtk_main_iteration();

	if ( ret == IDOK ) {
		str = strdup( gtk_entry_get_text( GTK_ENTRY( entry ) ) );
	}
	else{
		str = NULL;
	}

	gtk_grab_remove( dlg );
	gtk_widget_destroy( dlg );

	return str;
}

// =============================================================================
// NewProject dialog


static void DoNewProjectDlg_changed( GtkEntry *entry, gpointer data ){
	GtkWidget *pathlabel;

	pathlabel = GTK_WIDGET( GTK_LABEL( data ) );

	gtk_label_set_text( GTK_LABEL( pathlabel ), gtk_entry_get_text( GTK_ENTRY( entry ) ) );
}

char* DoNewProjectDlg( const char *path ){
	GtkWidget *dialog, *vbox, *hbox, *pathlabel, *label, *entry, *check, *content_area;
	int ret;
	gint response_id;
	char *str;
	GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;

	// start by a warning message
// mattn: URLs no longer valid
//  CString msg;
//  msg = "Are you sure you want a new project?\n";
//  msg += "Please note that creating a new project is not the prefered way to setup GtkRadiant for mod editing.\n";
//  msg += "Check http://www.qeradiant.com/faq/index.cgi?file=220 for more information";
//  if (gtk_MessageBox(NULL, msg.GetBuffer(), _("Confirm"), MB_YESNO, "http://www.qeradiant.com/faq/index.cgi?file=220" ) == IDNO)
//  {
//    return NULL;
//  }

	dialog = gtk_dialog_new_with_buttons( _( "New Project" ), NULL, flags, NULL );
	gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "OK" ), GTK_RESPONSE_OK );
	gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "Cancel" ), GTK_RESPONSE_CANCEL );

	content_area = gtk_dialog_get_content_area( GTK_DIALOG( dialog ) );

	vbox = gtk_box_new( GTK_ORIENTATION_VERTICAL, 5 );
	gtk_container_add( GTK_CONTAINER( content_area ), vbox );
	gtk_container_set_border_width( GTK_CONTAINER( vbox ), 5 );
	gtk_widget_show( vbox );

	label = gtk_label_new( _( "This will create a new directory beneath your\n"
							  "game path based on the project name you give." ) );
	gtk_box_pack_start( GTK_BOX( vbox ), label, FALSE, FALSE, 0 );
	gtk_widget_set_halign( label, GTK_ALIGN_START );
	gtk_widget_show( label );

	hbox = gtk_box_new( GTK_ORIENTATION_HORIZONTAL, 0 );
	gtk_box_pack_start( GTK_BOX( vbox ), hbox, TRUE, TRUE, 0 );
//	gtk_container_set_border_width( GTK_CONTAINER( hbox ), 5 );
	gtk_widget_show( hbox );

	label = gtk_label_new( path );
	gtk_box_pack_start( GTK_BOX( hbox ), label, FALSE, FALSE, 0 );
	gtk_widget_set_halign( label, GTK_ALIGN_END );
	gtk_widget_show( label );

	pathlabel = gtk_label_new( "" );
	gtk_box_pack_start( GTK_BOX( hbox ), pathlabel, TRUE, TRUE, 0 );
	gtk_widget_set_halign( pathlabel, GTK_ALIGN_START );
	gtk_widget_show( pathlabel );

	label = gtk_label_new( _( "Project name:" ) );
	gtk_box_pack_start( GTK_BOX( vbox ), label, TRUE, TRUE, 0 );
	gtk_widget_set_halign( label, GTK_ALIGN_START );
	gtk_widget_show( label );

	entry = gtk_entry_new();
	gtk_box_pack_start( GTK_BOX( vbox ), entry, TRUE, TRUE, 0 );
	gtk_widget_show( entry );

	check = gtk_check_button_new_with_label( _( "Include game dll files" ) );
	gtk_box_pack_start( GTK_BOX( vbox ), check, TRUE, TRUE, 0 );
	gtk_widget_set_sensitive( check, FALSE );
	gtk_widget_show( check );

	g_signal_connect( entry, "changed", G_CALLBACK( DoNewProjectDlg_changed ), pathlabel );

	gtk_widget_show( dialog );

	response_id = gtk_dialog_run( GTK_DIALOG( dialog ) );

	if( response_id == GTK_RESPONSE_OK ) 
	{
		str = strdup( gtk_entry_get_text( GTK_ENTRY( entry ) ) );
		ret = IDOK;
	} else {
		str = NULL;
		ret = IDCANCEL;
	}

	gtk_widget_destroy( dialog );

	return str;
}

// =============================================================================
// Text Editor dialog

// master window widget
static GtkWidget *text_editor = NULL;
static GtkWidget *text_widget; // slave, text widget from the gtk editor

static gint editor_delete( GtkWidget *widget, gpointer data ){
	if ( gtk_MessageBox( widget, _( "Close the shader editor ?" ), _( "Radiant" ), MB_YESNO ) == IDNO ) {
		return TRUE;
	}

	gtk_widget_hide( text_editor );

	return TRUE;
}

static void editor_save( GtkWidget *widget, gpointer data ){
	FILE *f = fopen( (char*)g_object_get_data( G_OBJECT( data ), "filename" ), "w" );
	gpointer text = g_object_get_data( G_OBJECT( data ), "text" );

	if ( f == NULL ) {
		gtk_MessageBox( GTK_WIDGET( data ), _( "Error saving file !" ) );
		return;
	}

	GtkTextBuffer *buffer = gtk_text_view_get_buffer( GTK_TEXT_VIEW( text ) );
	GtkTextIter start, end;
	gtk_text_buffer_get_bounds( buffer, &start, &end );
	char *str = gtk_text_buffer_get_text( buffer, &start, &end, FALSE );
	fwrite( str, 1, strlen( str ), f );
	fclose( f );
	g_free( str );
}

static void editor_close( GtkWidget *widget, gpointer data ){
	if ( gtk_MessageBox( text_editor, _( "Close the shader editor ?" ), _( "Radiant" ), MB_YESNO ) == IDNO ) {
		return;
	}

	gtk_widget_hide( text_editor );
}

// several attempts
#if 0
#ifdef _WIN32

HWND FindEditWindow(){
	return FindWindow( "TFormEditPadLite", NULL );
}

HWND FindEditWindow(){
	HWND hwnd = FindWindow( "TFormEditPadLite", NULL );
	if ( hwnd ) {
		hwnd = FindWindowEx( hwnd, NULL, "TPanel", NULL );
		if ( hwnd ) {
			hwnd = FindWindowEx( hwnd, NULL, "TPanel", NULL );
			if ( hwnd ) {
				hwnd = FindWindowEx( hwnd, NULL, "TEditPadEditor", NULL );
				if ( hwnd ) {
					hwnd = FindWindowEx( hwnd, NULL, "TWinControlProxy", NULL );
					return hwnd;
				}
			}
		}
	}
	return NULL;
}

HWND FindEditWindow(){
	HWND hwnd = FindWindow( "TFormEditPadLite", NULL );
	if ( hwnd ) {
		hwnd = FindWindowEx( hwnd, NULL, "TPanel", NULL );
		if ( hwnd ) {
			hwnd = FindWindowEx( hwnd, NULL, "TPanel", NULL );
			if ( hwnd ) {
				hwnd = FindWindowEx( hwnd, NULL, "TPanel", NULL );
				if ( hwnd ) {
					hwnd = FindWindowEx( hwnd, NULL, "TFrameSearchReplace", NULL );
					if ( hwnd ) {
						hwnd = FindWindowEx( hwnd, NULL, "TJGStringEditorControl", NULL );
						return hwnd;
					}
				}
			}
		}
	}
	return NULL;
}

HWND FindEditWindow(){
	HWND hwnd = FindWindow( "TEditPadForm", NULL );
	HWND hwndEdit = NULL;
	if ( hwnd != NULL ) {
		HWND hwndTab = FindWindowEx( hwnd, NULL, "TTabControl", NULL );
		if ( hwndTab != NULL ) {
			hwndEdit = FindWindowEx( hwndTab, NULL, "TRicherEdit", NULL );
		}
	}
	return hwndEdit;
}
#endif
#endif // #if 0

static void CreateGtkTextEditor(){
	GtkWidget *dlg;
	GtkWidget *vbox, *hbox, *button, *scr, *text;

	dlg = gtk_window_new( GTK_WINDOW_TOPLEVEL );

	g_signal_connect( dlg, "delete_event",
						G_CALLBACK( editor_delete ), NULL );
	gtk_window_set_default_size( GTK_WINDOW( dlg ), 600, 300 );

	vbox = gtk_box_new( GTK_ORIENTATION_VERTICAL, 5 );
	gtk_container_add( GTK_CONTAINER( dlg ), vbox );
	gtk_container_set_border_width( GTK_CONTAINER( vbox ), 5 );
	gtk_widget_show( vbox );

	scr = gtk_scrolled_window_new( NULL, NULL );
	gtk_box_pack_start( GTK_BOX( vbox ), scr, TRUE, TRUE, 0 );
	gtk_scrolled_window_set_policy( GTK_SCROLLED_WINDOW( scr ), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC );
	gtk_scrolled_window_set_shadow_type( GTK_SCROLLED_WINDOW( scr ), GTK_SHADOW_IN );
	gtk_widget_show( scr );

	text = gtk_text_view_new();
	gtk_container_add( GTK_CONTAINER( scr ), text );
	g_object_set_data( G_OBJECT( dlg ), "text", text );
	gtk_text_view_set_editable( GTK_TEXT_VIEW( text ), TRUE );
	gtk_widget_show( text );

	hbox = gtk_box_new( GTK_ORIENTATION_HORIZONTAL, 5 );
	gtk_box_pack_start( GTK_BOX( vbox ), hbox, FALSE, TRUE, 0 );
	gtk_widget_show( hbox );

	button = gtk_button_new_with_label( _( "Close" ) );
	gtk_box_pack_end( GTK_BOX( hbox ), button, FALSE, FALSE, 0 );
	g_signal_connect( button, "clicked",
						G_CALLBACK( editor_close ), dlg );
	gtk_widget_set_size_request( button, 60, -2 );
	gtk_widget_show( button );

	button = gtk_button_new_with_label( _( "Save" ) );
	gtk_widget_show( button );
	gtk_box_pack_end( GTK_BOX( hbox ), button, FALSE, FALSE, 0 );
	g_signal_connect( button, "clicked",
						G_CALLBACK( editor_save ), dlg );
	gtk_widget_set_size_request( button, 60, -2 );

	text_editor = dlg;
	text_widget = text;
}

static void DoGtkTextEditor( const char* filename, guint cursorpos ){
	if ( !text_editor ) {
		CreateGtkTextEditor(); // build it the first time we need it

	}
	// Load file
	FILE *f = fopen( filename, "r" );

	if ( f == NULL ) {
		Sys_Printf( "Unable to load file %s in shader editor.\n", filename );
		gtk_widget_hide( text_editor );
	}
	else
	{
		fseek( f, 0, SEEK_END );
		int len = ftell( f );
		void *buf = qmalloc( len );
		void *old_filename;

		rewind( f );
		fread( buf, 1, len, f );

		gtk_window_set_title( GTK_WINDOW( text_editor ), filename );

		GtkTextBuffer* text_buffer = gtk_text_view_get_buffer( GTK_TEXT_VIEW( text_widget ) );
		gtk_text_buffer_set_text( text_buffer, (char*)buf, len );

		old_filename = g_object_get_data( G_OBJECT( text_editor ), "filename" );
		if ( old_filename ) {
			free( old_filename );
		}
		g_object_set_data( G_OBJECT( text_editor ), "filename", strdup( filename ) );

		// trying to show later
		gtk_widget_show( text_editor );

#ifdef _WIN32
		while ( gtk_events_pending() )
			gtk_main_iteration();
#endif

		// only move the cursor if it's not exceeding the size..
		// NOTE: this is erroneous, cursorpos is the offset in bytes, not in characters
		// len is the max size in bytes, not in characters either, but the character count is below that limit..
		// thinking .. the difference between character count and byte count would be only because of CR/LF?
		{
			GtkTextIter text_iter;
			// character offset, not byte offset
			gtk_text_buffer_get_iter_at_offset( text_buffer, &text_iter, cursorpos );
			gtk_text_buffer_place_cursor( text_buffer, &text_iter );
		}

#ifdef _WIN32
		gtk_widget_queue_draw( text_widget );
#endif

		free( buf );
		fclose( f );
	}
}

void DoTextEditor( const char* filename, int cursorpos ){
	CString strEditCommand;
#ifdef _WIN32
	if ( g_PrefsDlg.m_bUseWin32Editor ) {
		HINSTANCE result;
		Sys_Printf( "opening file '%s'.\n", filename );
		result = ShellExecute( (HWND)GDK_WINDOW_HWND( gtk_widget_get_window( g_pParentWnd->m_pWidget ) ), "open", filename, NULL, NULL, SW_SHOW );
		if( (int)result <= 32 ) {
			const char *errstr;
			switch( (int)result ) {
				case SE_ERR_OOM:
				case 0:
					errstr = _( "The operating system is out of memory or resources." );
					break;
				case ERROR_FILE_NOT_FOUND:
				//case SE_ERR_FNF:
					errstr = _( "The specified file was not found." );
					break;
				case SE_ERR_NOASSOC: 
					errstr = _( "There is no application associated with the given file name extension." );
					break;
				case ERROR_PATH_NOT_FOUND:
				//case SE_ERR_PNF:
					errstr = _( "The specified path was not found." );
					break;
				default:
					errstr = "";
					break;
			}
			Sys_Printf( "Failed to open file '%s'. %s\n", filename, errstr );
		}
		return;
	}
#else
	// check if a custom editor is set
	if ( ( g_PrefsDlg.m_bUseCustomEditor ) && ( g_PrefsDlg.m_strEditorCommand.GetLength() > 0 ) ) {
		strEditCommand = g_PrefsDlg.m_strEditorCommand;
		strEditCommand += " \"";
		strEditCommand += filename;
		strEditCommand += "\"";

		Sys_Printf( "Launching: %s\n", strEditCommand.GetBuffer() );
		// note: linux does not return false if the command failed so it will assume success
		if ( Q_Exec( NULL, (char *)strEditCommand.GetBuffer(), NULL, true ) == false ) {
			Sys_FPrintf( SYS_WRN, "Warning: Failed to execute %s, using default\n", strEditCommand.GetBuffer() );
		}
		else
		{
			// the command (appeared) to run successfully, no need to do anything more
			return;
		}
	}
#endif

	DoGtkTextEditor( filename, cursorpos );

	// old win32 code with EditPad bindings, broken
#if 0
	strEditCommand = g_strAppPath.GetBuffer();
	strEditCommand += "editpad.exe";
	strEditCommand += " \"";
	strEditCommand += filename;
	strEditCommand += "\"";
	if ( Q_Exec( NULL, (char *)strEditCommand.GetBuffer(), NULL, true ) == false ) {
		Sys_FPrintf( SYS_WRN, "WARNING: Gtk shader editor is not fully functional on windows in general and unstable on win98 in particular.\n" );
		Sys_FPrintf( SYS_WRN, "  you can use EditPad instead (install it in Radiant's directory): http://www.qeradiant.com/?data=files&files_dir=18\n" );
		DoGtkTextEditor( filename, cursorpos );
	}
	else
	{
		// TTimo: we used to call Delay here, to continue processing messages. But it seems to induce a lot of instabilities.
		// so now the user will simply have to wait.
		Sleep( 1500 );

		// now grab the edit window and scroll to the shader we want to edit
		HWND hwndEdit = FindEditWindow();

		if ( hwndEdit != NULL ) {
			PostMessage( hwndEdit, EM_SETSEL, cursorpos, cursorpos );
		}
		else{
			Sys_Printf( "Unable to load shader editor.\n" );
		}
	}
#endif
}

// =============================================================================
// Light Intensity dialog

int DoLightIntensityDlg( int *intensity ){
	GtkWidget *dialog, *vbox, *hbox, *label, *content_area, *spinbutton, *button;
	GtkAdjustment *adj;
	gint response_id;
	int ret;
	GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;

	dialog = gtk_dialog_new_with_buttons( _( "Light intensity" ), NULL, flags, NULL );	

	GtkAccelGroup *accel_group = gtk_accel_group_new();
	gtk_window_add_accel_group( GTK_WINDOW( dialog ), accel_group );

	button = gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "OK" ), GTK_RESPONSE_OK );
	gtk_widget_add_accelerator( button, "clicked", accel_group,
								GDK_KEY_Return, (GdkModifierType)0, GTK_ACCEL_VISIBLE );
	button = gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "Cancel" ), GTK_RESPONSE_CANCEL );
	gtk_widget_add_accelerator( button, "clicked", accel_group,
								GDK_KEY_Escape, (GdkModifierType)0, GTK_ACCEL_VISIBLE );

	content_area = gtk_dialog_get_content_area( GTK_DIALOG( dialog ) );

	vbox = gtk_box_new( GTK_ORIENTATION_VERTICAL, 5 );
	gtk_container_add( GTK_CONTAINER( content_area ), vbox );
	gtk_widget_show( vbox );

	label = gtk_label_new( _( "ESC for default, ENTER to validate" ) );
	gtk_box_pack_start( GTK_BOX( vbox ), label, FALSE, FALSE, 0 );
	gtk_widget_set_halign( label, GTK_ALIGN_START );
	gtk_widget_show( label );

	hbox = gtk_box_new( GTK_ORIENTATION_HORIZONTAL, 5 );
	gtk_box_pack_start( GTK_BOX( vbox ), hbox, TRUE, TRUE, 0 );
	gtk_container_set_border_width( GTK_CONTAINER( hbox ), 5 );
	gtk_widget_show( hbox );

	adj = gtk_adjustment_new( *intensity, 0, G_MAXINT, 1, 10, 0 );
	spinbutton = gtk_spin_button_new( GTK_ADJUSTMENT( adj ), 1, 0 );
	gtk_box_pack_start( GTK_BOX( hbox ), spinbutton, TRUE, TRUE, 0 );
	gtk_spin_button_set_wrap( GTK_SPIN_BUTTON( spinbutton ), FALSE );
	gtk_spin_button_set_numeric( GTK_SPIN_BUTTON( spinbutton ), TRUE );
	gtk_widget_set_hexpand( spinbutton, TRUE );
	gtk_widget_show( spinbutton );
	g_object_set( spinbutton, "xalign", 1.0, NULL );

	gtk_spin_button_set_value( GTK_SPIN_BUTTON( spinbutton ), *intensity );

	gtk_widget_show( dialog );

	response_id = gtk_dialog_run( GTK_DIALOG( dialog ) );

	if( response_id == GTK_RESPONSE_OK ) 
	{
		*intensity = gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON( spinbutton ) );
		ret = IDOK;
	} else {
		ret = IDCANCEL;
	}

	gtk_widget_destroy( dialog );

	return ret;
}

static void OnReplace_clicked( GtkButton *button, gpointer data )
{
	gboolean bSelectedOnly, bForce;
	GtkToggleButton *check;
	GtkWidget *dialog, *entry, *find_combo, *replace_combo;
	const gchar *strFind, *strReplace;

	dialog = GTK_WIDGET( data );

	check = GTK_TOGGLE_BUTTON( g_object_get_data( G_OBJECT( dialog ), "bSelectedOnly_check" ) );
	bSelectedOnly = gtk_toggle_button_get_active( check );

	check = GTK_TOGGLE_BUTTON( g_object_get_data( G_OBJECT( dialog ), "bForce_check" ) );
	bForce = gtk_toggle_button_get_active( check );

	find_combo = GTK_WIDGET( g_object_get_data( G_OBJECT( dialog ), "find_combo" ) );
	entry = gtk_bin_get_child( GTK_BIN( find_combo ) );
	strFind = gtk_entry_get_text( GTK_ENTRY( entry ) );

	replace_combo = GTK_WIDGET( g_object_get_data( G_OBJECT( dialog ), "replace_combo" ) );
	entry = gtk_bin_get_child( GTK_BIN( replace_combo ) );
	strReplace = gtk_entry_get_text( GTK_ENTRY( entry ) );

	FindReplaceTextures( strFind, strReplace, bSelectedOnly, bForce, FALSE );
}
static void OnFind_clicked( GtkButton *button, gpointer data )
{
	gboolean bSelectedOnly;
	GtkToggleButton *check;
	GtkWidget *dialog, *entry, *find_combo, *replace_combo;
	const gchar *strFind, *strReplace;

	dialog = GTK_WIDGET( data );

	check = GTK_TOGGLE_BUTTON( g_object_get_data( G_OBJECT( dialog ), "bSelectedOnly_check" ) );
	bSelectedOnly = gtk_toggle_button_get_active( check );

	find_combo = GTK_WIDGET( g_object_get_data( G_OBJECT( dialog ), "find_combo" ) );
	entry = gtk_bin_get_child( GTK_BIN( find_combo ) );
	strFind = gtk_entry_get_text( GTK_ENTRY( entry ) );

	replace_combo = GTK_WIDGET( g_object_get_data( G_OBJECT( dialog ), "replace_combo" ) );
	entry = gtk_bin_get_child( GTK_BIN( replace_combo ) );
	strReplace = gtk_entry_get_text( GTK_ENTRY( entry ) );
	
	FindReplaceTextures( strFind, strReplace, bSelectedOnly, FALSE, TRUE );
}

void DoFindReplaceTexturesDialog()
{
	GtkWidget *dialog, *content_area, *combo, *hbox;
	GtkWidget *vbox, *table, *label, *button, *find_button, *replace_button;
	GtkWidget *find_combo, *replace_combo, *check;
	GtkSizeGroup *button_group;
	GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;

	dialog = gtk_dialog_new_with_buttons( _( "Find / Replace Texture(s)" ), NULL, flags, NULL );

	GtkAccelGroup *accel_group = gtk_accel_group_new();
	gtk_window_add_accel_group( GTK_WINDOW( dialog ), accel_group );

	button = gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "Close" ), GTK_RESPONSE_OK );
	gtk_widget_hide( button );
	gtk_widget_add_accelerator( button, "clicked", accel_group, GDK_KEY_Escape, (GdkModifierType)0, GTK_ACCEL_VISIBLE );

	content_area = gtk_dialog_get_content_area( GTK_DIALOG( dialog ) );

	vbox = gtk_box_new( GTK_ORIENTATION_VERTICAL, 5 );
	gtk_box_pack_start( GTK_BOX( content_area ), vbox, FALSE, FALSE, 0 );
	gtk_widget_show( vbox );

	table = gtk_grid_new();
	gtk_box_pack_start( GTK_BOX( vbox ), table, TRUE, TRUE, 0 );
	gtk_grid_set_row_spacing( GTK_GRID( table ), 5 );
	gtk_grid_set_column_spacing( GTK_GRID( table ), 5 );
	gtk_widget_show( table );

	label = gtk_label_new( "Find:" );
	gtk_grid_attach( GTK_GRID( table ), label, 0, 0, 1, 1 );
	gtk_widget_set_halign( label, GTK_ALIGN_START );
	gtk_widget_show( label );

	label = gtk_label_new( "Replace:" );
	gtk_grid_attach( GTK_GRID( table ), label, 0, 1, 1, 1 );
	gtk_widget_set_halign( label, GTK_ALIGN_START );
	gtk_widget_show( label );

	find_combo = combo = gtk_combo_box_text_new_with_entry();
	gtk_grid_attach( GTK_GRID( table ), combo, 1, 0, 1, 1 );
	gtk_widget_set_hexpand( combo, TRUE );
	gtk_widget_show( combo );
	g_object_set_data( G_OBJECT( dialog ), "find_combo", find_combo );

	replace_combo = combo = gtk_combo_box_text_new_with_entry();
	gtk_grid_attach( GTK_GRID( table ), combo, 1, 1, 1, 1 );
	gtk_widget_set_hexpand( combo, TRUE );
	gtk_widget_show( combo );
	g_object_set_data( G_OBJECT( dialog ), "replace_combo", replace_combo );


	check = gtk_check_button_new_with_label( "Use selected brushes only" );
	gtk_box_pack_start( GTK_BOX( vbox ), check, TRUE, TRUE, 0 );
	gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( check ), FALSE );
	g_object_set_data( G_OBJECT( dialog ), "bSelectedOnly_check", check );
	gtk_widget_show( check );

	check = gtk_check_button_new_with_label( "Replace everywhere (selected/active), don't test against Find" );
	gtk_box_pack_start( GTK_BOX( vbox ), check, TRUE, TRUE, 0 );
	gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( check ), FALSE );
	g_object_set_data( G_OBJECT( dialog ), "bForce_check", check );
	gtk_widget_show( check );

/*	check = gtk_check_button_new_with_label( "Live updates from Texture/Camera windows" );
	gtk_widget_show( check );
	gtk_box_pack_start( GTK_BOX( vbox ), check, TRUE, TRUE, 0 );
	gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( check ), TRUE );
	g_object_set_data( G_OBJECT( dialog ), "bLive_check", check );
*/
	hbox = gtk_box_new( GTK_ORIENTATION_HORIZONTAL, 5 );
	gtk_box_pack_start( GTK_BOX( vbox ), hbox, TRUE, TRUE, 0 );
	gtk_widget_set_halign( hbox, GTK_ALIGN_END );
	gtk_widget_show( hbox );

	find_button = button = gtk_button_new_with_label( _( "Find" ) );
	gtk_box_pack_start( GTK_BOX( hbox ), button, FALSE, FALSE, 0 );
	gtk_widget_set_halign( button, GTK_ALIGN_END );
	gtk_widget_show( button );
	g_signal_connect( button, "clicked", G_CALLBACK( OnFind_clicked ), dialog );

	replace_button = button = gtk_button_new_with_label( _( "Replace" ) );
	gtk_box_pack_start( GTK_BOX( hbox ), button, FALSE, FALSE, 0 );
	gtk_widget_set_halign( button, GTK_ALIGN_END );
	gtk_widget_show( button );
	g_signal_connect( button, "clicked", G_CALLBACK( OnReplace_clicked ), dialog );

	button_group = gtk_size_group_new( GTK_SIZE_GROUP_BOTH );
	gtk_size_group_add_widget( button_group, find_button );
	gtk_size_group_add_widget( button_group, replace_button );
	g_object_unref( button_group );


	gtk_widget_show( dialog );
}