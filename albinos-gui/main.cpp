# include "gtk-layer-shell/gtk-layer-shell.h"
# include <gtk/gtk.h>

static void activate(GtkApplication* app, void *_data)
{
    (void)_data;

    GtkWindow *gtk_window = GTK_WINDOW (gtk_application_window_new (app));

    gtk_layer_init_for_window(gtk_window);
    gtk_layer_set_layer(gtk_window, GTK_LAYER_SHELL_LAYER_BOTTOM);
    gtk_layer_auto_exclusive_zone_enable(gtk_window);
    gtk_layer_set_keyboard_interactivity(gtk_window, TRUE);

    GtkWidget *box = gtk_grid_new();
    GtkWidget *addButton = gtk_button_new_with_label("Add");
    GtkWidget *createButton = gtk_button_new_with_label("Create");
    GtkWidget *removeButton = gtk_button_new_with_label("Remove");
    GtkWidget *item_list_1 = gtk_label_new("config 1");
    GtkWidget *item_list_2 = gtk_label_new("config 2");
    GtkWidget *item_list_3 = gtk_label_new("config 3");
    GtkWidget *list = gtk_list_box_new();
    GtkWidget *configDisplay = gtk_tree_view_new();

    GtkWidget *horizSep = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_list_box_prepend(GTK_LIST_BOX(list), item_list_3);
    gtk_list_box_prepend(GTK_LIST_BOX(list), item_list_2);
    gtk_list_box_prepend(GTK_LIST_BOX(list), item_list_1);
    gtk_grid_attach(GTK_GRID(box), createButton, 0, 0, 1, 1);
    gtk_grid_attach_next_to(GTK_GRID(box), addButton, createButton, GTK_POS_RIGHT, 1, 1);
    gtk_grid_attach_next_to(GTK_GRID(box), removeButton, addButton, GTK_POS_RIGHT, 1, 1);
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
