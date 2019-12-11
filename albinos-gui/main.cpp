# include "gtk-layer-shell/gtk-layer-shell.h"
# include <gtk/gtk.h>
# include "ConfigManager.hpp"

static ConfigManager *configManager;

enum {
    CONFIRM,
    CANCEL
};

static bool getTextEntry(std::string const &txtLabel, std::string &entry)
{
    bool valid = false;
    GtkWidget *askWin = gtk_dialog_new();
    GtkWidget *entryField = gtk_entry_new();
    GtkWidget *label = gtk_label_new(txtLabel.c_str());
    gtk_dialog_add_action_widget(GTK_DIALOG(askWin), label, CONFIRM);
    gtk_dialog_add_action_widget(GTK_DIALOG(askWin), entryField, CONFIRM);
    gtk_dialog_add_button(GTK_DIALOG(askWin), "Ok", CONFIRM);
    gtk_dialog_add_button(GTK_DIALOG(askWin), "Cancel", CANCEL);
    gtk_widget_show_all(GTK_WIDGET (askWin));
    if (gtk_dialog_run(GTK_DIALOG(askWin)) == CONFIRM)
        valid = true;
    entry = gtk_entry_get_text(GTK_ENTRY(entryField));
    gtk_widget_destroy(GTK_WIDGET(askWin));
    return valid;
}

static void clickConfig(GtkListBox *box, GtkListBoxRow *row, gpointer data)
{
    if (!row)
        configManager->fetchConfig("");
    else
        configManager->fetchConfig(gtk_label_get_label(GTK_LABEL(gtk_bin_get_child(GTK_BIN(row)))));
}

static void createConfig(GtkWidget *widget, gpointer data)
{
    (void)data;
    std::string name;

    if (getTextEntry("Config name", name)) {
        configManager->addNewConfig(name);
        gtk_widget_show_all(GTK_WIDGET (gtk_widget_get_parent(widget)));
    }
}

static void addConfig(GtkWidget *widget, gpointer data)
{
    (void)data;
    std::string key;

    if (getTextEntry("Config key", key)) {
        std::string name = configManager->addExistingConfig(key);
        gtk_widget_show_all(GTK_WIDGET (gtk_widget_get_parent(widget)));
    }
}

static void deleteConfig(GtkWidget *widget, gpointer data)
{
    (void)data;
    GtkWidget *askWin = gtk_dialog_new();
    GtkWidget *deleteList = gtk_combo_box_text_new();
    for (auto elem = gtk_container_get_children(GTK_CONTAINER(configManager->getList())) ; elem ; elem = elem->next) {
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(deleteList), gtk_label_get_label(GTK_LABEL(gtk_bin_get_child(GTK_BIN(elem->data)))));
    }
    gtk_dialog_add_action_widget(GTK_DIALOG(askWin), deleteList, CONFIRM);
    gtk_dialog_add_button(GTK_DIALOG(askWin), "Ok", CONFIRM);
    gtk_dialog_add_button(GTK_DIALOG(askWin), "Cancel", CANCEL);
    gtk_widget_show_all(GTK_WIDGET (askWin));

    if (gtk_dialog_run(GTK_DIALOG(askWin)) == CONFIRM && gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(deleteList))) {
        configManager->deleteConfig(gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(deleteList)));
        gtk_widget_show_all(GTK_WIDGET (gtk_widget_get_parent(widget)));
    }
    gtk_widget_destroy(GTK_WIDGET(askWin));
}

static void manageKeyboardEvents(GtkWidget *widget, GdkEventKey *evt, gpointer data)
{
    if (evt->keyval == GDK_KEY_Delete) {
        GtkWidget *listBox = GTK_WIDGET(data);
        configManager->removeConfig(gtk_label_get_label(GTK_LABEL(gtk_bin_get_child(GTK_BIN(gtk_list_box_get_selected_row(GTK_LIST_BOX(listBox)))))));
    }
}

static void activate(GtkApplication* app, void *_data)
{
    (void)_data;

    GtkWindow *gtk_window = GTK_WINDOW (gtk_application_window_new (app));

    gtk_widget_set_events(GTK_WIDGET(gtk_window), GDK_KEY_PRESS_MASK);

    gtk_layer_init_for_window(gtk_window);
    gtk_layer_set_layer(gtk_window, GTK_LAYER_SHELL_LAYER_BOTTOM);
    gtk_layer_auto_exclusive_zone_enable(gtk_window);
    gtk_layer_set_keyboard_interactivity(gtk_window, TRUE);

    GtkWidget *box = gtk_grid_new();
    GtkWidget *list = gtk_list_box_new();
    g_signal_connect(G_OBJECT(list), "row-selected", G_CALLBACK(clickConfig), nullptr);
    GtkWidget *configDisplay = gtk_grid_new();
    gtk_grid_insert_column(GTK_GRID(configDisplay), 0);
    gtk_grid_insert_column(GTK_GRID(configDisplay), 1);
    configManager = new ConfigManager(list, configDisplay);

    g_signal_connect(G_OBJECT(gtk_window), "key_press_event", G_CALLBACK(manageKeyboardEvents), list);

    GtkWidget *addButton = gtk_button_new_with_label("Add");
    g_signal_connect(G_OBJECT(addButton), "clicked", G_CALLBACK(addConfig), nullptr);
    GtkWidget *createButton = gtk_button_new_with_label("Create");
    g_signal_connect(G_OBJECT(createButton), "clicked", G_CALLBACK(createConfig), nullptr);
    GtkWidget *deleteButton = gtk_button_new_with_label("Delete");
    g_signal_connect(G_OBJECT(deleteButton), "clicked", G_CALLBACK(deleteConfig), nullptr);

    GtkWidget *horizSep = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_grid_attach(GTK_GRID(box), createButton, 0, 0, 1, 1);
    gtk_grid_attach_next_to(GTK_GRID(box), addButton, createButton, GTK_POS_RIGHT, 1, 1);
    gtk_grid_attach_next_to(GTK_GRID(box), deleteButton, addButton, GTK_POS_RIGHT, 1, 1);
    gtk_grid_attach_next_to(GTK_GRID(box), horizSep, createButton, GTK_POS_BOTTOM, 30, 1);
    gtk_grid_attach_next_to(GTK_GRID(box), configDisplay, horizSep, GTK_POS_BOTTOM, 30, 20);
    gtk_grid_attach_next_to(GTK_GRID(box), list, configDisplay, GTK_POS_RIGHT, 1, 20);
    gtk_container_add (GTK_CONTAINER (gtk_window), box);
    gtk_container_set_border_width (GTK_CONTAINER (gtk_window), 12);
    gtk_widget_show_all (GTK_WIDGET (gtk_window));
}

int main(int ac, char **av)
{
    GtkApplication *app = gtk_application_new("sh.wmww.albinos-gui", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    int status = g_application_run(G_APPLICATION(app), ac, av);
    g_object_unref(app);
    return status;
}
