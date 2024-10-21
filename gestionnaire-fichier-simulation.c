#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <string.h>

/*  TIPE 2024-2025, MPI Lycée Louis Thuillier: Cardot Clément et William Guerrin-Garnier

Objectifs: simuler un gestionnaire de fichiers
    - accès à une base de donnés
    - opération sur la base (ajouter un fichier, le renommer)
    - 

*/

//══════════════════════════════════════════ FONCTION POUR 'ftype' ══════════════════════════════════════════

typedef struct {
    int id;
    char nom[128];
    char extension[16];
    char adresse[256];
    long offset;
    long longueur;
} ftype;

void nom_data(sqlite3 *db, ftype *fichier) {
    char requete[512];
    snprintf(requete, sizeof(requete), "SELECT nom FROM fichiers WHERE id = %d;", fichier->id);
    
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, requete, -1, &stmt, NULL) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            const char *nom = (const char *)sqlite3_column_text(stmt, 0);
            strncpy(fichier->nom, nom, sizeof(fichier->nom));
        }
    }
    sqlite3_finalize(stmt);
}

void extension_data(sqlite3 *db, ftype *fichier) {
    char requete[512];
    snprintf(requete, sizeof(requete), "SELECT extension FROM fichiers WHERE id = %d;", fichier->id);
    
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, requete, -1, &stmt, NULL) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            const char *extension = (const char *)sqlite3_column_text(stmt, 0);
            strncpy(fichier->extension, extension, sizeof(fichier->extension));
        }
    }
    sqlite3_finalize(stmt);
}

void adresse_data(sqlite3 *db, ftype *fichier) {
    char requete[512];
    snprintf(requete, sizeof(requete), "SELECT adresse FROM fichiers WHERE id = %d;", fichier->id);
    
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, requete, -1, &stmt, NULL) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            const char *adresse = (const char *)sqlite3_column_text(stmt, 0);
            strncpy(fichier->adresse, adresse, sizeof(fichier->adresse));
        }
    }
    sqlite3_finalize(stmt);
}

void offset_data(sqlite3 *db, ftype *fichier) {
    char requete[512];
    snprintf(requete, sizeof(requete), "SELECT offset FROM fichiers WHERE id = %d;", fichier->id);
    
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, requete, -1, &stmt, NULL) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            fichier->offset = sqlite3_column_int64(stmt, 0);
        }
    }
    sqlite3_finalize(stmt);
}

void longueur_data(sqlite3 *db, ftype *fichier) {
    char requete[512];
    snprintf(requete, sizeof(requete), "SELECT longueur FROM fichiers WHERE id = %d;", fichier->id);
    
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, requete, -1, &stmt, NULL) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            fichier->longueur = sqlite3_column_int64(stmt, 0);
        }
    }
    sqlite3_finalize(stmt);
}

//═══════════════════════════════════════════ FONCTION SQL / HEXA ═══════════════════════════════════════════

void extraire_nom_et_extension(ftype *fichier) {
    const char *nom_fichier = strrchr(fichier->adresse, '/');  // Trouver le dernier '/'
    if (!nom_fichier) {
        nom_fichier = fichier->adresse; // Si aucun '/' trouvé, utiliser tout le adresse
    } else {
        nom_fichier++; // Passer le '/'
    }

    const char *ext = strrchr(nom_fichier, '.');
    if (ext) {
        strcpy(fichier->extension, ext + 1); // Copier l'extension sans le point
        strncpy(fichier->nom, nom_fichier, ext - nom_fichier); // Copier le nom sans l'extension
        fichier->nom[ext - nom_fichier] = '\0'; // Terminer la chaîne de caractères
    } else {
        strcpy(fichier->nom, nom_fichier); // Pas d'extension trouvée
        strcpy(fichier->extension, "");
    }
}

int convertir_fichier_en_hexa(ftype *fichier) {
    FILE *fichier_bin = fopen(fichier->adresse, "rb");
    if (!fichier_bin) {
        fprintf(stderr, "Erreur d'ouverture du fichier : %s\n", fichier->adresse);
        return -1;
    }
    FILE *fichier_hexa = fopen("data_hexa.txt", "a");
    if (!fichier_hexa) {
        fprintf(stderr, "Erreur d'ouverture du fichier data_hexa.txt\n");
        fclose(fichier_bin);
        return -1;
    }
    unsigned char octet;
    while (fread(&octet, 1, 1, fichier_bin) == 1) {
        fprintf(fichier_hexa, "%02X", octet);
    }
    fclose(fichier_bin);
    fclose(fichier_hexa);
    return 0;
}

int supprime_fichier(const char *adresse) {
    if (remove(adresse) == 0) {
        printf(" Fichier '%s' supprimé avec succès.\n", adresse);
        return 0;
    } else {
        perror(" Erreur de suppression du fichier");
        return -1;
    }
}

int ajouter_fichier(sqlite3 *database, ftype *fichier) {
    char *message_erreur = 0;
    char requete[512];
    long offset;
    long longueur = 0;

    // Extraire le nom et l'extension
    extraire_nom_et_extension(fichier);

    // Ouvrir le fichier en hexadécimal et déterminer l'offset
    FILE *fichier_hexa = fopen("fichier_hexadecimal.txt", "a");
    if (!fichier_hexa) {
        fprintf(stderr, "Erreur d'ouverture du fichier fichier_hexadecimal.txt\n");
        return -1;
    }
    offset = ftell(fichier_hexa); // Obtenir l'offset actuel

    FILE *fichier_bin = fopen(fichier->adresse, "rb");
    if (!fichier_bin) {
        fprintf(stderr, "Erreur d'ouverture du fichier : %s\n", fichier->adresse);
        fclose(fichier_hexa);
        return -1;
    }
    
    unsigned char octet;
    while (fread(&octet, 1, 1, fichier_bin) == 1) {
        fprintf(fichier_hexa, "%02X", octet);
        longueur++;
    }
    fclose(fichier_bin);
    fclose(fichier_hexa);

    // Mettre à jour l'offset et la longueur dans la structure
    fichier->offset = offset;
    fichier->longueur = longueur;

    // Insérer dans la base de données avec l'offset et la longueur
    snprintf(requete, sizeof(requete), 
         "INSERT INTO fichiers (nom, extension, adresse) VALUES ('%s', '%s', '%ld:%ld');",
         fichier->nom, fichier->extension, fichier->offset, fichier->longueur);

    int resultat = sqlite3_exec(database, requete, 0, 0, &message_erreur);
    if (resultat != SQLITE_OK) {
        fprintf(stderr, "Erreur d'insertion : %s\n", message_erreur);
        sqlite3_free(message_erreur);
        return resultat;
    }

    fprintf(stdout, "Fichier '%s' avec extension '%s' ajouté avec succès.\n\n", fichier->nom, fichier->extension);

    if (supprime_fichier(fichier->adresse) != 0) {
        fprintf(stderr, "Erreur lors de la suppression du fichier : %s\n", fichier->adresse);
    }
    return SQLITE_OK;
}

int ouvrir_data(sqlite3 *database, ftype *fichier) {
    char requete[256];
    char *message_erreur = 0;
    sqlite3_stmt *stmt;
    
    snprintf(requete, sizeof(requete), "SELECT adresse FROM fichiers WHERE id = '%d';", fichier->id);
    
    if (sqlite3_prepare_v2(database, requete, -1, &stmt, 0) != SQLITE_OK) {
        fprintf(stderr, "Erreur de préparation de la requête : %s\n", sqlite3_errmsg(database));
        return -1;
    }

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        // Lire l'adresse sous forme de chaîne et extraire l'offset et la longueur
        const char *adresse = (const char *)sqlite3_column_text(stmt, 0);
        sscanf(adresse, "%ld:%ld", &(fichier->offset), &(fichier->longueur));
    } else {
        fprintf(stderr, "Fichier non trouvé dans la base de données.\n");
        sqlite3_finalize(stmt);
        return -1;
    }
    sqlite3_finalize(stmt);

    // Lecture du fichier hexa et reconstruction du fichier
    FILE *fichier_hexa = fopen("fichier_hexadecimal.txt", "r");
    if (!fichier_hexa) {
        fprintf(stderr, "Erreur d'ouverture du fichier fichier_hexadecimal.txt\n");
        return -1;
    }

    FILE *fichier_sortie = fopen(fichier->nom, "wb");
    if (!fichier_sortie) {
        fprintf(stderr, "Erreur de création du fichier : %s.%s\n", fichier->nom, fichier->extension);
        fclose(fichier_hexa);
        return -1;
    }

    fseek(fichier_hexa, fichier->offset, SEEK_SET);
    for (long i = 0; i < fichier->longueur; i++) {
        unsigned int octet;
        fscanf(fichier_hexa, "%02X", &octet);
        fputc(octet, fichier_sortie);
    }

    fclose(fichier_sortie);
    fclose(fichier_hexa);
    return 0;
}

int supprimer(sqlite3 *database, ftype *fichier) {
    char requete[256];
    char *message_erreur = 0;
    sqlite3_stmt *stmt;
    long offset, longueur;
    
    // 1. Récupérer l'offset et la longueur du fichier à supprimer
    snprintf(requete, sizeof(requete), "SELECT adresse FROM fichiers WHERE id = '%d';", fichier->id);
    if (sqlite3_prepare_v2(database, requete, -1, &stmt, 0) != SQLITE_OK) {
        fprintf(stderr, "Erreur de préparation de la requête : %s\n", sqlite3_errmsg(database));
        return -1;
    }

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        // Lire l'adresse sous forme de chaîne et extraire l'offset et la longueur
        const char *adresse = (const char *)sqlite3_column_text(stmt, 0);
        sscanf(adresse, "%ld:%ld", &offset, &longueur);
    } else {
        fprintf(stderr, "Fichier non trouvé dans la base de données.\n");
        sqlite3_finalize(stmt);
        return -1;
    }
    sqlite3_finalize(stmt);

    // 2. Supprimer l'entrée de la base de données
    snprintf(requete, sizeof(requete), "DELETE FROM fichiers WHERE id = '%d';", fichier->id);
    int resultat = sqlite3_exec(database, requete, 0, 0, &message_erreur);
    if (resultat != SQLITE_OK) {
        fprintf(stderr, "Erreur de suppression : %s\n", message_erreur);
        sqlite3_free(message_erreur);
        return resultat;
    }

    // 3. Lire le contenu du fichier hexadecimal dans un buffer (nécessaire pour le réécrire ensuite)
    FILE *fichier_hexa = fopen("fichier_hexadecimal.txt", "r");
    if (!fichier_hexa) {
        fprintf(stderr, "Erreur d'ouverture du fichier fichier_hexadecimal.txt\n");
        return -1;
    }

    fseek(fichier_hexa, 0, SEEK_END);
    long taille_totale = ftell(fichier_hexa);
    rewind(fichier_hexa);

    char *buffer = malloc(taille_totale + 1);
    if (!buffer) {
        fprintf(stderr, "Erreur d'allocation mémoire.\n");
        fclose(fichier_hexa);
        return -1;
    }
    fread(buffer, 1, taille_totale, fichier_hexa);
    fclose(fichier_hexa);

    // 4. Supprimer les données du fichier à supprimer en compactant le buffer
    long nouvelle_taille = taille_totale - (longueur * 2); // Chaque octet = 2 caractères hexadécimaux
    char *nouveau_buffer = malloc(nouvelle_taille + 1);
    if (!nouveau_buffer) {
        fprintf(stderr, "Erreur d'allocation mémoire.\n");
        free(buffer);
        return -1;
    }

    // Copier les parties avant et après l'offset à supprimer
    memcpy(nouveau_buffer, buffer, offset);
    memcpy(nouveau_buffer + offset, buffer + offset + (longueur * 2), taille_totale - offset - (longueur * 2));

    free(buffer);

    // 5. Réécrire le fichier hexadecimal avec le nouveau contenu
    fichier_hexa = fopen("fichier_hexadecimal.txt", "w");
    if (!fichier_hexa) {
        fprintf(stderr, "Erreur d'ouverture en écriture du fichier fichier_hexadecimal.txt\n");
        free(nouveau_buffer);
        return -1;
    }

    fwrite(nouveau_buffer, 1, nouvelle_taille, fichier_hexa);
    fclose(fichier_hexa);
    free(nouveau_buffer);

    // 6. Mettre à jour les offsets des autres fichiers dans la base de données
    snprintf(requete, sizeof(requete), "SELECT id, adresse FROM fichiers;");
    if (sqlite3_prepare_v2(database, requete, -1, &stmt, 0) != SQLITE_OK) {
        fprintf(stderr, "Erreur de préparation de la requête : %s\n", sqlite3_errmsg(database));
        return -1;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        const char *adresse = (const char *)sqlite3_column_text(stmt, 1);
        long fichier_offset, fichier_longueur;
        sscanf(adresse, "%ld:%ld", &fichier_offset, &fichier_longueur);

        // Si l'offset du fichier se situe après celui qu'on vient de supprimer, on le décale
        if (fichier_offset > offset) {
            fichier_offset -= (longueur * 2); // Mettre à jour l'offset

            // Mettre à jour l'entrée dans la base de données
            snprintf(requete, sizeof(requete), 
                     "UPDATE fichiers SET adresse = '%ld:%ld' WHERE id = %d;", 
                     fichier_offset, fichier_longueur, id);

            resultat = sqlite3_exec(database, requete, 0, 0, &message_erreur);
            if (resultat != SQLITE_OK) {
                fprintf(stderr, "Erreur de mise à jour : %s\n", message_erreur);
                sqlite3_free(message_erreur);
                sqlite3_finalize(stmt);
                return resultat;
            }
        }
    }
    sqlite3_finalize(stmt);

    printf("Fichier '%s' supprimé avec succès.\n", fichier->nom);
    return 0;
}



//══════════════════════════════════════ FONCTION AFFICHAGE ET INTERFACE ══════════════════════════════════════
// (┼ ─ ├ ┬ ┴ │ └ ┐ ┘ ┌ ┤ ╬ ═ ╠ ╦ ╩ ║ ╚ ╗ ╝ ╔ ╣)

int fonction_de_rappel(void *NonUtilise, int nbColonnes, char **valeurs, char **nomsColonnes) {
    printf("├──────┼───────────┼─────────────────────┤\n   %s       %s       %s\n", valeurs[0], valeurs[1], valeurs[2]);
    return 0;
}

void affiche_tab(sqlite3 *database, char *requete_selection) {
    int resultat;
    char *message_erreur = 0;
    printf("\n\n\n\n\n════════════════════════════════════════════\n");
    printf("┌──────┬───────────┬─────────────────────┐\n");
    printf("│  id  │ extension │         nom         │\n");
    resultat = sqlite3_exec(database, requete_selection, fonction_de_rappel, 0, &message_erreur);
    printf("└──────┴───────────┴─────────────────────┘");
    if (resultat != SQLITE_OK) {
        fprintf(stderr, " Erreur lors de la sélection des données : %s\n", message_erreur);
        sqlite3_free(message_erreur);
    }
}

void ls_database(sqlite3 *database) {
    char *requete_selection = "SELECT * FROM fichiers;";
    affiche_tab(database, requete_selection);
}

void recherche(sqlite3 *database, ftype *fichier) {
    char requete_selection[256];
    snprintf(requete_selection, sizeof(requete_selection), "SELECT * FROM fichiers WHERE extension LIKE '%s';", fichier->extension);
    //snprintf: printf dans un char* tampon (permet de recevoir le "%s")
    affiche_tab(database, requete_selection);
}

void renommer(sqlite3 *database, ftype *fichier) {
    int resultat;
    char *message_erreur = 0;
    char requete_selection[256];
    snprintf(requete_selection, sizeof(requete_selection), "UPDATE fichiers SET nom = '%s' WHERE id = %d;", fichier->nom, fichier->id);
    //snprintf: printf dans un char* tampon (permet de recevoir le "%s")
    resultat = sqlite3_exec(database, requete_selection, fonction_de_rappel, 0, &message_erreur);
    if (resultat != SQLITE_OK) {
        fprintf(stderr, " Erreur lors de la sélection des données : %s\n", message_erreur);
        sqlite3_free(message_erreur);
    }
    snprintf(requete_selection, sizeof(requete_selection), "SELECT * FROM fichiers WHERE id = %d;", fichier->id);
    affiche_tab(database, requete_selection);
}

void interface(sqlite3 *database) {
    int choix;
    ftype *fichier = malloc(sizeof(ftype));

    while (1) {
        printf("\n═══════════════════ Menu ═══════════════════\n");
        printf(" 1. Afficher la base de données\n");
        printf(" 2. Ajouter un fichier\n");
        printf(" 3. Ouvrir un fichier\n");
        printf(" 4. Rechercher des fichiers par extension\n");
        printf(" 5. Renommer un fichier\n");
        printf(" 6. Supprimer un fichier\n");
        printf(" 7. Quitter\n");
        printf("════════════════════════════════════════════\n");
                
        printf(" Entrez votre choix : ");
        scanf("%d", &choix);
        getchar();  // Pour consommer le retour à la ligne laissé par scanf

        switch (choix) {
            case 1:
                ls_database(database);
                break;

            case 2:
                printf("\n\n================== Ajout ===================\n");
                printf(" adresse du fichier à ajouter : ");
                fgets(fichier->adresse, sizeof(fichier->adresse), stdin);
                fichier->adresse[strcspn(fichier->adresse, "\n")] = 0;  // Enlever le retour à la ligne
                if (ajouter_fichier(database, fichier) == 0) {
                    printf("\n\n Fichier ajouté avec succès.\n\n");
                } else {
                    printf("\n\n Erreur lors de l'ajout du fichier.\n\n");
                }
                break;

            case 3: 
                printf("\n\n================== Ouvrir ===================\n");
                printf(" ID du fichier à ouvrir : ");
                scanf("%d", &(fichier->id));
                getchar();  // Pour consommer le retour à la ligne laissé par scanf
                // Remplir le nom, l'extension, l'adresse, et l'offset
                nom_data(database, fichier);
                extension_data(database, fichier);
                adresse_data(database, fichier);
                offset_data(database, fichier);
                
                // Après récupération des données, essayez d'ouvrir le fichier
                if (ouvrir_data(database, fichier) == 0) {
                    printf("\n\n Fichier ouvert et créé avec succès.\n\n");
                } else {
                    printf("\n\n Erreur lors de l'ouverture du fichier.\n\n");
                }
                break;

            case 4:
                printf("\n\n================ Recherche =================\n");
                printf(" Extension des fichiers à rechercher : ");
                fgets(fichier->extension, sizeof(fichier->extension), stdin);
                fichier->extension[strcspn(fichier->extension, "\n")] = 0;  
                recherche(database, fichier);
                break;


            case 5: 
                printf("\n\n================ Renommer ==================\n");
                printf(" ID du fichier à renommer : ");
                scanf("%d", &(fichier->id));
                getchar();  // Pour consommer le retour à la ligne laissé par scanf
                printf(" Nouveau nom du fichier : ");
                fgets(fichier->nom, sizeof(fichier->nom), stdin);
                fichier->nom[strcspn(fichier->nom, "\n")] = 0;  
                renommer(database, fichier);
                break;

            case 6:
                printf("\n\n================== Supprimer =================\n");
                printf(" ID du fichier à supprimer : ");
                scanf("%d", &(fichier->id));
                getchar();  // Pour consommer le retour à la ligne laissé par scanf
                if (supprimer(database, fichier) == 0) {
                    printf("\n\n Fichier supprimé avec succès.\n\n");
                } else {
                    printf("\n\n Erreur lors de la suppression du fichier.\n\n");
                }
                break;

            case 7:
                printf("\n════════════════════════════════════════════\n");
                printf("                 Au revoir !\n\n");
                free(fichier);
                return;

            default:
                printf(" Choix invalide. Veuillez réessayer.\n");
        }
    }
}



//═══════════════════════════════════════════════════ MAIN ══════════════════════════════════════════════════

int main() {
    sqlite3 *database;
    char *message_erreur = 0;
    int resultat;
    resultat = sqlite3_open("database.db", &database);
    if (resultat) {
        fprintf(stderr, "Erreur d'ouverture de la base de données : %s\n", sqlite3_errmsg(database));
        return resultat;
    }
    const char *requete_creation_table = 
        "CREATE TABLE IF NOT EXISTS fichiers("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "extension TEXT NOT NULL,"
        "nom TEXT NOT NULL,"
        "adresse TEXT NOT NULL);";

    resultat = sqlite3_exec(database, requete_creation_table, 0, 0, &message_erreur);
    if (resultat != SQLITE_OK) {
        fprintf(stderr, "Erreur de création de table : %s\n", message_erreur);
        sqlite3_free(message_erreur);
        sqlite3_close(database);
        return resultat;
    } else {
        fprintf(stdout, "Table 'fichiers' créée ou déjà existante.\n");
    }

    interface(database);

    sqlite3_close(database);

    return 0;
}