#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*  TIPE 2024-2025, MPI Lycée Louis Thuillier: Cardot Clément et William Guerrin-Garnier

Objectif: Prendre en argument le "fichier_hexadecimal.txt" et créer tous les fichiers
          possiblement stockés dedans, selon le type fourni.

    - Il faut donc un texte: "fichier_hexadecimal.txt" rempli de données de fichiers
    - Utilisation possible du "gestionnaire-fichier-simulation.c" pour créer ce texte
*/

typedef struct {
    const char *nom;
    const unsigned char *debut;
    const unsigned char *fin;
    size_t taille_debut;
    size_t taille_fin;
} Extension;

int caractereHexaEnInt(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

unsigned char hex_vers_octet(const char *hexa) {
    return (caractereHexaEnInt(hexa[0]) << 4) | caractereHexaEnInt(hexa[1]);
}

void extraire_fichier(const char *fichier_hex, const Extension *extension) {
    FILE *fichier = fopen(fichier_hex, "r");
    if (fichier == NULL) {
        perror("Erreur lors de l'ouverture du fichier texte hexadécimal");
        return;
    }

    char tampon[3];
    unsigned char *donnees_fichier = NULL;
    size_t taille_fichier = 0;
    int compteur_fichier = 0;
    int dans_fichier = 0;

    while (fscanf(fichier, "%2s", tampon) == 1) {
        unsigned char octet = hex_vers_octet(tampon);

        if (!dans_fichier && octet == extension->debut[0]) {
            int match = 1;
            for (size_t i = 1; i < extension->taille_debut; i++) {
                fscanf(fichier, "%2s", tampon);
                if (hex_vers_octet(tampon) != extension->debut[i]) {
                    match = 0;
                    break;
                }
            }
            if (match) {
                dans_fichier = 1;
                donnees_fichier = malloc(extension->taille_debut);
                memcpy(donnees_fichier, extension->debut, extension->taille_debut);
                taille_fichier = extension->taille_debut;
                continue;
            }
        }

        if (dans_fichier) {
            donnees_fichier = realloc(donnees_fichier, taille_fichier + 1);
            donnees_fichier[taille_fichier++] = octet;

            if (taille_fichier >= extension->taille_fin) {
                int match = 1;
                for (size_t i = 0; i < extension->taille_fin; i++) {
                    if (donnees_fichier[taille_fichier - extension->taille_fin + i] != extension->fin[i]) {
                        match = 0;
                        break;
                    }
                }
                if (match) {
                    char nom_fichier[50];
                    sprintf(nom_fichier, "fichier%d.%s", ++compteur_fichier, extension->nom);
                    FILE *fichier_sortie = fopen(nom_fichier, "wb");
                    if (fichier_sortie != NULL) {
                        fwrite(donnees_fichier, 1, taille_fichier, fichier_sortie);
                        fclose(fichier_sortie);
                        printf("Fichier extrait : %s\n", nom_fichier);
                    } else {
                        perror("Erreur lors de la création du fichier");
                    }

                    free(donnees_fichier);
                    donnees_fichier = NULL;
                    taille_fichier = 0;
                    dans_fichier = 0;
                }
            }
        }
    }

    fclose(fichier);
    if (donnees_fichier) free(donnees_fichier);
}

int main() {
    const unsigned char jpeg_debut[] = {0xFF, 0xD8};
    const unsigned char jpeg_fin[] = {0xFF, 0xD9};
    Extension jpeg_extension = { "jpeg", jpeg_debut, jpeg_fin, sizeof(jpeg_debut), sizeof(jpeg_fin) };

    const unsigned char txt_debut[] = {0x00};
    const unsigned char txt_fin[] = {0x0A};
    Extension txt_extension = { "txt", txt_debut, txt_fin, sizeof(txt_debut), sizeof(txt_fin) };

    const unsigned char png_debut[] = {0x89, 0x50, 0x4E, 0x47};
    const unsigned char png_fin[] = {0x49, 0x45, 0x4E, 0x44, 0xAE, 0x42, 0x60, 0x82};
    Extension png_extension = { "png", png_debut, png_fin, sizeof(png_debut), sizeof(png_fin) };

    extraire_fichier("fichier_hexadecimal.txt", &jpeg_extension);
    // extraire_fichier("fichier_hexadecimal.txt", &txt_extension);
    extraire_fichier("fichier_hexadecimal.txt", &png_extension);

    return 0;
}
