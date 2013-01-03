/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*-

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.
  
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.
  
   You should have received a copy of the GNU General Public
   License along with this program; if not, write to the
   Free Software Foundation, Inc., 51 Franklin Street - Suite 500,
   Boston, MA 02110-1335, USA.

*/

#include "nemo-action.h"

G_DEFINE_TYPE (NemoAction, nemo_action,
	       GTK_TYPE_ACTION);

static void     nemo_action_init       (NemoAction      *action);

static void     nemo_action_class_init (NemoActionClass *klass);

static void     nemo_action_get_property  (GObject                    *object,
                                           guint                       param_id,
                                           GValue                     *value,
                                           GParamSpec                 *pspec);

static void     nemo_action_set_property  (GObject                    *object,
                                           guint                       param_id,
                                           const GValue               *value,
                                           GParamSpec                 *pspec);

static void     nemo_action_finalize (GObject *gobject);

static   gpointer parent_class;

#define ACTION_FILE_GROUP "Nemo Action"

#define KEY_ACTIVE "Active"
#define KEY_NAME "Name"
#define KEY_COMMENT "Comment"
#define KEY_EXEC "Exec"
#define KEY_ICON "Icon"
#define KEY_SELECTION "Selection"
#define KEY_EXTENSIONS "Extensions"

enum 
{
  PROP_0,
  PROP_KEY_FILE,
  PROP_SELECTION_TYPE,
  PROP_EXTENSIONS,
  PROP_EXT_LENGTH,
  PROP_EXEC,
  PROP_PARENT_DIR,
  PROP_USE_PARENT_DIR,
  PROP_ORIG_LABEL
};

static void
nemo_action_init (NemoAction *action)
{
    action->key_file = NULL;
    action->selection_type = SELECTION_SINGLE;
    action->extensions = NULL;
    action->ext_length = 0;
    action->exec = NULL;
    action->parent_dir = NULL;
    action->use_parent_dir = FALSE;
}

static void
nemo_action_class_init (NemoActionClass *klass)
{
	GtkActionClass *action_class = GTK_ACTION_CLASS (klass);
    GObjectClass         *object_class = G_OBJECT_CLASS(klass);
    parent_class           = g_type_class_peek_parent (klass);
    object_class->finalize = nemo_action_finalize;
    object_class->set_property = nemo_action_set_property;
    object_class->get_property = nemo_action_get_property;


    g_object_class_install_property (object_class,
                                     PROP_KEY_FILE,
                                     g_param_spec_pointer ("key-file",
                                                           "Key File",
                                                           "The key file associated with this action",
                                                           G_PARAM_READWRITE)
                                     );

    g_object_class_install_property (object_class,
                                     PROP_SELECTION_TYPE,
                                     g_param_spec_int ("selection-type",
                                                        "Selection Type",
                                                        "The action selection type",
                                                        SELECTION_SINGLE,
                                                        SELECTION_NONE,
                                                        SELECTION_SINGLE,
                                                        G_PARAM_READWRITE)
                                     );

    g_object_class_install_property (object_class,
                                     PROP_EXTENSIONS,
                                     g_param_spec_pointer ("extensions",
                                                           "Extensions",
                                                           "String array of file extensions",
                                                           G_PARAM_READWRITE)
                                     );

    g_object_class_install_property (object_class,
                                     PROP_EXT_LENGTH,
                                     g_param_spec_int ("ext-length",
                                                        "Extensions Length",
                                                        "Number of extensions",
                                                        0,
                                                        999,
                                                        0,
                                                        G_PARAM_READWRITE)
                                     );

    g_object_class_install_property (object_class,
                                     PROP_EXEC,
                                     g_param_spec_string ("exec",
                                                          "Executable String",
                                                          "The command line to run",
                                                          NULL,
                                                          G_PARAM_READWRITE)
                                     );

    g_object_class_install_property (object_class,
                                     PROP_PARENT_DIR,
                                     g_param_spec_string ("parent-dir",
                                                          "Parent directory",
                                                          "The directory the action file resides in",
                                                          NULL,
                                                          G_PARAM_READWRITE)
                                     );
    g_object_class_install_property (object_class,
                                     PROP_USE_PARENT_DIR,
                                     g_param_spec_boolean ("use-parent-dir",
                                                           "Use Parent Directory",
                                                           "Execute using the full action path",
                                                           FALSE,
                                                           G_PARAM_READWRITE)
                                     );
    g_object_class_install_property (object_class,
                                     PROP_ORIG_LABEL,
                                     g_param_spec_string ("orig-label",
                                                          "Original label string",
                                                          "The starting label - with token",
                                                          NULL,
                                                          G_PARAM_READWRITE)
                                     );
}

GtkAction *
nemo_action_new (const gchar *name, 
                 NemoFile    *file)
{
    GKeyFile *key_file = g_key_file_new();

    gchar *path = g_filename_from_uri (nemo_file_get_uri (file), NULL, NULL);

    g_key_file_load_from_file (key_file, path, G_KEY_FILE_NONE, NULL);

    if (!g_key_file_has_group (key_file, ACTION_FILE_GROUP))
        return NULL;

    if (g_key_file_has_key (key_file, ACTION_FILE_GROUP, KEY_ACTIVE, NULL)) {
        if (!g_key_file_get_boolean (key_file, ACTION_FILE_GROUP, KEY_ACTIVE, NULL))
            return NULL;
    }

    gchar *orig_label = g_key_file_get_locale_string (key_file,
                                                      ACTION_FILE_GROUP,
                                                      KEY_NAME,
                                                      NULL,
                                                      NULL);

    gchar *tooltip = g_key_file_get_locale_string (key_file, 
                                                   ACTION_FILE_GROUP,
                                                   KEY_COMMENT,
                                                   NULL,
                                                   NULL);

    gchar *icon_name = g_key_file_get_string (key_file,
                                              ACTION_FILE_GROUP,
                                              KEY_ICON,
                                              NULL);

    gchar *exec_raw = g_key_file_get_string (key_file,
                                             ACTION_FILE_GROUP,
                                             KEY_EXEC,
                                             NULL);

    gchar *selection_string = g_key_file_get_string (key_file,
                                                     ACTION_FILE_GROUP,
                                                     KEY_SELECTION,
                                                     NULL);

    SelectionType type;

    if (g_strcmp0 (selection_string, SELECTION_SINGLE_KEY) == 0)
        type = SELECTION_SINGLE;
    else if (g_strcmp0 (selection_string, SELECTION_MULTIPLE_KEY) == 0)
        type = SELECTION_MULTIPLE;
    else if (g_strcmp0 (selection_string, SELECTION_ANY_KEY) == 0)
        type = SELECTION_ANY;
    else if (g_strcmp0 (selection_string, SELECTION_NONE_KEY) == 0)
        type = SELECTION_NONE;
    else
        type = SELECTION_SINGLE;

    g_free (selection_string);

    gsize count;

    gchar **ext = g_key_file_get_string_list (key_file,
                                              ACTION_FILE_GROUP,
                                              KEY_EXTENSIONS,
                                              &count,
                                              NULL);

    if (orig_label == NULL || exec_raw == NULL || ext == NULL) {
        g_printerr ("An action definition requires, at minimum, "
                    "a Label field, and Exec field, and an Extensions field.\n"
                    "Check the %s file for missing fields.\n", path);
        g_free (path);
        return NULL;
    }

    gchar *exec = NULL;
    gboolean use_parent_dir = FALSE;

    if (g_str_has_prefix (exec_raw, "<") && g_str_has_suffix (exec_raw, ">")) {
        gchar **split = g_strsplit_set (exec_raw, "<>", 3);
        exec = g_strdup (split[1]);
        use_parent_dir = TRUE;
        g_free (split);
    } else {
        exec = g_strdup (exec_raw);
    }

    g_free (exec_raw);

    gchar *parent_dir = g_filename_from_uri (nemo_file_get_parent_uri (file), NULL, NULL);

    return g_object_new (NEMO_TYPE_ACTION,
                         "name", name,
                         "key-file", key_file,
                         "label", orig_label,
                         "tooltip", tooltip,
                         "icon-name", icon_name,
                         "exec", exec,
                         "selection-type", type,
                         "ext-length", count,
                         "extensions", ext,
                         "parent-dir", parent_dir,
                         "use-parent-dir", use_parent_dir,
                         "orig-label", orig_label,
                          NULL);
}

static void
nemo_action_finalize (GObject *object)
{
    NemoAction *action = NEMO_ACTION (object);

    g_key_file_free (action->key_file);
    g_strfreev (action->extensions);
    g_free (action->exec);
    g_free (action->parent_dir);
    g_free (action->orig_label);

    G_OBJECT_CLASS (parent_class)->finalize (object);
}


static void
nemo_action_set_property (GObject         *object,
             guint            prop_id,
             const GValue    *value,
             GParamSpec      *pspec)
{
  NemoAction *action;
  
  action = NEMO_ACTION (object);

  switch (prop_id)
    {
    case PROP_KEY_FILE:
      action->key_file = g_value_get_pointer (value);
      break;
    case PROP_SELECTION_TYPE:
      action->selection_type = g_value_get_int (value);
      break;
    case PROP_EXTENSIONS:
      action->extensions = g_value_get_pointer (value);
      break;
    case PROP_EXT_LENGTH:
      action->ext_length = g_value_get_int (value);
      break;
    case PROP_EXEC:
      nemo_action_set_exec (action, g_value_get_string (value));
      break;
    case PROP_PARENT_DIR:
      nemo_action_set_parent_dir (action, g_value_get_string (value));
      break;
    case PROP_USE_PARENT_DIR:
      action->use_parent_dir = g_value_get_boolean (value);
      break;
    case PROP_ORIG_LABEL:
      nemo_action_set_orig_label (action, g_value_get_string (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
nemo_action_get_property (GObject    *object,
             guint       prop_id,
             GValue     *value,
             GParamSpec *pspec)
{
  NemoAction *action;

  action = NEMO_ACTION (object);

  switch (prop_id)
    {
    case PROP_KEY_FILE:
      g_value_set_pointer (value, action->key_file);
      break;
    case PROP_SELECTION_TYPE:
      g_value_set_int (value, action->selection_type);
      break;
    case PROP_EXTENSIONS:
      g_value_set_pointer (value, action->extensions);
      break;
    case PROP_EXT_LENGTH:
      g_value_set_int (value, action->ext_length);
      break;
    case PROP_EXEC:
      g_value_set_string (value, action->exec);
      break;
    case PROP_PARENT_DIR:
      g_value_set_string (value, action->parent_dir);
      break;
    case PROP_USE_PARENT_DIR:
      g_value_set_boolean (value, action->use_parent_dir);
      break;
    case PROP_ORIG_LABEL:
      g_value_set_string (value, action->orig_label);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

void
nemo_action_activate (NemoAction *action, GList *selection)
{
    GList *token, *iter;
    GList *arg_list = NULL;
    gchar **exec_args;
    gint exec_arg_count;

    g_shell_parse_argv (action->exec, &exec_arg_count, &exec_args, NULL);

    int i;

    /* First, build the basic arg list based on action->exec */

    for (i = 0; i < exec_arg_count; i++) {
        arg_list = g_list_append (arg_list, g_strdup (exec_args[i]));
    }

    /* Now, find the file list token in the argument list, and insert our selection
     * in front of it
     */

    for (token = arg_list; token != NULL; token = token->next) {
        if (g_strcmp0 (token->data, TOKEN_EXEC_FILE_LIST) == 0)
            break;
    }

    if (token != NULL) {
        for (iter = selection; iter != NULL; iter = iter->next) {
            arg_list = g_list_insert_before (arg_list, token,
                                             g_filename_from_uri (nemo_file_get_uri (NEMO_FILE (iter->data)), NULL, NULL));
        }
        arg_list = g_list_delete_link (arg_list, token);
    }

    /* Now make our arg vector array for passing to the g_spawn_async function */

    gchar *argv[g_list_length (arg_list)+1];
    i = 0;
    if (action->use_parent_dir) {
        argv[i] = g_build_filename (action->parent_dir, g_strdup (arg_list->data), NULL);
    } else {
        argv[i] = g_strdup (arg_list->data);
    }
    i++;

    for (iter = arg_list->next; iter != NULL; iter = iter->next) {
        argv[i] = g_strdup (iter->data);
        i++;
    }

    argv[i] = NULL;

    /* Finally spawn the command */

    g_spawn_async (NULL, argv, NULL, G_SPAWN_SEARCH_PATH,
                   NULL, NULL, NULL, NULL);

    nemo_file_list_free (selection);
    g_list_free_full (arg_list, g_free);
    g_strfreev (exec_args);
}

SelectionType
nemo_action_get_selection_type (NemoAction *action)
{
    return action->selection_type;
}

gchar **
nemo_action_get_extension_list (NemoAction *action)
{
    return action->extensions;
}

guint
nemo_action_get_extension_count (NemoAction *action)
{
    return action->ext_length;
}

void
nemo_action_set_exec (NemoAction *action, const gchar *exec)
{
    gchar *tmp;

    tmp = action->exec;
    action->exec = g_strdup (exec);
    g_free (tmp);
}

void
nemo_action_set_parent_dir (NemoAction *action, const gchar *parent_dir)
{
    gchar *tmp;

    tmp = action->parent_dir;
    action->parent_dir = g_strdup (parent_dir);
    g_free (tmp);
}

void
nemo_action_set_orig_label (NemoAction *action, const gchar *orig_label)
{
    gchar *tmp;

    tmp = action->orig_label;
    action->orig_label = g_strdup (orig_label);
    g_free (tmp);
}

gchar *
nemo_action_get_orig_label (NemoAction *action)
{
    return action->orig_label;
}

static gboolean
test_string_for_label_token (const gchar *string)
{
    return g_strstr_len (string, -1, TOKEN_LABEL_FILE_NAME) != NULL;
}

void
nemo_action_set_label (NemoAction *action, NemoFile *file) {

    const gchar *orig_label = nemo_action_get_orig_label (action);

    if (!test_string_for_label_token (orig_label) || file == NULL ||
        action->selection_type > SELECTION_MULTIPLE) {
        gtk_action_set_label (GTK_ACTION (action), orig_label);
        return;
    }

    gchar *display_name = nemo_file_get_display_name (file);

    gchar **split = g_strsplit (orig_label, TOKEN_LABEL_FILE_NAME, 2);

    gchar *new_label = g_strconcat (split[0], display_name, split[1], NULL);

    gtk_action_set_label (GTK_ACTION (action), new_label);

    g_strfreev (split);
    g_free (display_name);
    g_free (new_label);
}
