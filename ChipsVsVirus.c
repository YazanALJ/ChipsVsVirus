#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>

#define LARGEUR_PL 24
#define LONGEUR_PL 7

typedef struct chips {
    int type_i;
    char type_c;
    int life;
    int line;
    int position;
    int price;
    struct chips *next;
} Chips, *Liste_Chips;

typedef struct virus {
    int type_i;
    char type_c;
    int life;
    int line;
    int position;
    int speed;
    int turn;
    struct virus *next;
    struct virus *next_line;
    struct virus *prev_line;
} Virus, *Liste_Virus;

typedef struct {
    Virus *virus;
    Chips *chips;
    int turn;
    int money;
} Game;

/* t (type), line et position sont choisies par le joueur, soit, on suppose
   qu'ils sont valides, soit, on crée des fonctions de validation pour ça. */
Chips *alloueChip(char type, int line, int position) {
    Chips *chip = (Chips *) malloc(sizeof(Chips));
    if (chip != NULL) {
        chip->line = line;
        chip->position = position;
        chip->next = NULL;
        chip->type_c = type;
        switch (type) {
            case 'A': /* ALU, tourelle de base */
                chip->type_i = 1;
                chip->life = 2;
                chip->price = 100;
                break;
            case 'R': /* RAM, ralentie les virus */
                chip->type_i = 2;
                chip->life = 1;
                chip->price = 300;
                break;
            case 'F': /* Firewall, bloque les virus */
                chip->type_i = 3;
                chip->life = 10;
                chip->price = 200;
                break;
            case 'P': /* PMMU, effet laser, dégâts à tous les virus sur une ligne */
                chip->type_i = 4;
                chip->life = 1;
                chip->price = 400;
                break;
            case 'C': /* CPU, spread triple shot */
                chip->type_i = 5;
                chip->life = 2;
                chip->price = 250;
                break;
            default:;
        }
    }
    return chip;
}

void insererChip(Liste_Chips *lst_c, char type, int line, int position) {
    Chips *new_chip = alloueChip(type, line, position);
    if (new_chip == NULL) {
        fprintf(stderr, "Erreur d'allocation d'un chip/plus de memoire\n");
        exit(1);
    }

    if (*lst_c == NULL) {
        *lst_c = new_chip;
    } else {
        Chips *last_chip = *lst_c;
        while (last_chip->next != NULL) {
            last_chip = last_chip->next;
        }
        last_chip->next = new_chip;
    }
}

void supprimechip(Liste_Chips *lst_c, Chips *chip) {
    Liste_Chips index = *lst_c;
    if (*lst_c == NULL)
        return;
    if (*lst_c == chip) {
        *lst_c = (*lst_c)->next;
        // chip->next = NULL; inutile
        free(chip);
    } else {
        while (index->next != NULL && index->next != chip)
            index = index->next;
        if (index->next != NULL) {
            index->next = chip->next;
            // chip->next = NULL; inutile
            free(chip);
        }
    }
}

void freeListChips(Liste_Chips *lst_c) {
    Chips *courant = *lst_c;
    Chips *suivant;
    while (courant != NULL) {
        suivant = courant->next;
        free(courant);
        courant = suivant;
    }
    *lst_c = NULL;
}

Virus *alloueVirus(char type, int line, int position, int turn) {
    Virus *vi = (Virus *) malloc(sizeof(Virus));
    if (vi != NULL) {
        vi->line = line;
        vi->position = position;
        vi->turn = turn;
        vi->next = NULL;
        vi->next_line = NULL;
        vi->prev_line = NULL;
        vi->type_c = type;
        switch (type) {
            case 'E': // Floating point exception, base virus
                vi->type_i = 1;
                vi->life = 6;
                vi->speed = 1;
                break;
            case 'M': // Corruption de mémoire, peu résistant mais rapide
                vi->type_i = 2;
                vi->life = 4;
                vi->speed = 4;
                break;
            case 'B': // bug silencieux, résilient, dangereux, relativement
                // rapide
                vi->type_i = 3;
                vi->life = 12;
                vi->speed = 2;
                break;
            case 'D': // DDOS, healer, heals one virus behind and in front,
                // range differs
                vi->type_i = 4;
                vi->life = 4;
                vi->speed = 1;
                break;
            case 'S': // Seg fault, consomme la vie du virus derrière lui,
                // peut le tuer
                vi->type_i = 5;
                vi->life = 1;
                vi->speed = 1;
                break;
            default:;
        }
    }
    return vi;
}

int virus_type_to_life(char type) {
    switch (type) {
        case 'E':
            return 6;
        case 'M':
            return 4;
        case 'B':
            return 12;
        case 'D':
            return 4;
        case 'S':
            return 1;
        default:;
    }
    return 0;
}

void insererVirus(Liste_Virus *lst_v, char type, int line, int position, int
turn) {
    Virus *new_virus = alloueVirus(type, line, position, turn);
    if (new_virus == NULL) {
        fprintf(stderr, "Erreur d'allocation d'un virus/plus de memoire\n");
        exit(1);
    }

    if (*lst_v == NULL) {
        *lst_v = new_virus;
    } else {
        Virus *last_virus = *lst_v;
        while (last_virus->next != NULL) {
            last_virus = last_virus->next;
        }
        last_virus->next = new_virus;
    }
}

void freeListVirus(Liste_Virus *lst_v) {
    Virus *courant = *lst_v;
    Virus *suivant;
    while (courant != NULL) {
        suivant = courant->next;
        free(courant);
        courant = suivant;
    }
    *lst_v = NULL;
}

void supprimevirus(Liste_Virus *lst_v, Virus *virus) {
    Liste_Virus index = *lst_v;
    if (*lst_v == NULL)
        return;
    if (*lst_v == virus) {
        *lst_v = (*lst_v)->next; // Si (*lst_v)->next == NULL alors virus
        // next_line == NULL aussi.
        if (virus->next_line != NULL) {
            virus->next_line->prev_line = NULL;

        }
        free(virus);
    } else {
        while (index->next != NULL && index->next != virus)
            index = index->next;
        if (index->next != NULL) {
            index->next = virus->next;
            if (virus->next_line != NULL)
                virus->next_line->prev_line = virus->prev_line;
            if (virus->prev_line != NULL)
                virus->prev_line->next_line = virus->next_line;
            free(virus);
        }
    }
}

void doublechainage(Liste_Virus lst_v) {
    Virus *current;
    for (; lst_v != NULL; lst_v = lst_v->next) {
        current = lst_v->next;
        for (; current != NULL; current = current->next) {
            if (lst_v->line == current->line) {
                lst_v->next_line = current;
                current->prev_line = lst_v;
                break;
            }
        }
    }
}


void initialise_Vlst_from_file(Game *mygame, char fname[], int *maxturn) {
    int tour, ligne;
    char type;
    Liste_Virus *lst_v;
    FILE *f;
    lst_v = &(mygame->virus);
    f = fopen(fname, "r");
    if (f == NULL) {
        fprintf(stderr, "Erreur dans l'ouverture du fichier\n");
        exit(1);
    }
    fscanf(f, "%d", &(mygame->money));

    while (fscanf(f, "%d %d %c", &tour, &ligne, &type) != EOF) {
        insererVirus(lst_v, type, ligne, tour + LARGEUR_PL, tour);
        if (tour > *maxturn)
            *maxturn = tour;
    }

    fclose(f);

    doublechainage(mygame->virus);
}

void initialise_tab_wave(Liste_Virus lst_v, int maxturn, char
tab[LONGEUR_PL][maxturn]) {
    int i, j;
    for (i = 0; i < LONGEUR_PL; i++) {
        for (j = 0; j < maxturn; j++) {
            tab[i][j] = '.';
        }
    }
    for (; lst_v != NULL; lst_v = lst_v->next) {
        tab[lst_v->line - 1][lst_v->turn - 1] = lst_v->type_c;
    }
}

void affiche_wave(int maxturn, char tab[LONGEUR_PL][maxturn]) {
    int i, j;
    printf("Here is the wave of problems approaching...\n");
    for (i = 0; i < LONGEUR_PL; i++) {
        printf("|%d", i + 1);
        for (j = 0; j < maxturn; j++) {
            if (strchr("EMBDS", tab[i][j]) != NULL) {
                printf(" %2d%c", virus_type_to_life(tab[i][j]), tab[i][j]);
            } else
                printf("   %c", tab[i][j]);
        }
        printf("\n");
    }
}

void init_maj_game_tab(char game_tab[LONGEUR_PL][LARGEUR_PL], Game *game) {
    Liste_Virus lst_v = game->virus;
    Liste_Chips lst_c = game->chips;
    int i, j;
    for (i = 0; i < LONGEUR_PL; i++) {
        for (j = 0; j < LARGEUR_PL; j++) {
            game_tab[i][j] = '.';
        }
    }
    for (; lst_v != NULL; lst_v = lst_v->next) {
        if (1 <= lst_v->position && lst_v->position <= LARGEUR_PL) {
            game_tab[lst_v->line - 1][lst_v->position - 1] = lst_v->type_c;
        }
    }

    for (; lst_c != NULL; lst_c = lst_c->next) {
        game_tab[lst_c->line - 1][lst_c->position - 1] = lst_c->type_c;
    }
}

int coord_to_life(void *lst, int line, int position, size_t size) {
    if (size == sizeof(Chips)) {
        Liste_Chips lst_c = (Liste_Chips) lst;
        for (; lst_c != NULL; lst_c = lst_c->next) {
            if (lst_c->line == line && lst_c->position == position)
                return lst_c->life;
        }
    } else if (size == sizeof(Virus)) {
        Liste_Virus lst_v = (Liste_Virus) lst;
        for (; lst_v != NULL; lst_v = lst_v->next) {
            if (lst_v->line == line && lst_v->position == position)
                return lst_v->life;
        }
    }
    return -1; // To silence a warning.
}

void affiche_game_tab(char game_tab[LONGEUR_PL][LARGEUR_PL], Game *game) {
    int i, j, pdv;

    printf("Game turn : %d\n", game->turn);
    for (i = 0; i < LONGEUR_PL; i++) {
        printf("|%d ", i + 1);
        for (j = 0; j < LARGEUR_PL; j++) {
            if (strchr("ARFPC", game_tab[i][j]) != NULL) {
                pdv = coord_to_life(game->chips, i + 1, j + 1, sizeof(Chips));
                printf("%*c%d", 4 - (int) (floor(log10(abs(pdv))) + 1),
                       game_tab[i][j], pdv);
            }  // Don't ask...
            else if (strchr("EMBDS", game_tab[i][j]) != NULL)
                printf(" %2d%c", coord_to_life(game->virus,
                                               i + 1, j + 1,
                                               sizeof(Virus)), game_tab[i][j]);
            else
                printf("   %c", game_tab[i][j]);
        }
        printf("\n");
    }
}

int type_to_price(char t) {
    switch (t) {
        case 'A':
            return 100;
        case 'R':
            return 300;
        case 'F':
            return 200;
        case 'P':
            return 400;
        case 'C':
            return 250;
        default:
            return -1;
    }
}

int place_taken(Game *game, int line, int position) {
    Liste_Chips lst_c = game->chips;
    Liste_Virus lst_v = game->virus;
    for (; lst_c != NULL; lst_c = lst_c->next) {
        if (lst_c->line == line && lst_c->position == position)
            return 1;
    }
    for (; lst_v != NULL; lst_v = lst_v->next) {
        if (lst_v->line == line && lst_v->position == position)
            return 1;
    }
    return 0;
}

int in_plateau(int line, int position) {
    if ((1 <= line && line <= LONGEUR_PL) && (1 <= position && position <=
                                                               LARGEUR_PL))
        return 1;
    return 0;
}

void choix_et_selection(Game *game, int maxturn, char
tab[LONGEUR_PL][maxturn], char game_tab[LONGEUR_PL][LARGEUR_PL]) {
    char c;
    int line, position, taken;

    affiche_wave(maxturn, tab);
    printf("What chips do you want to deploy ?\n");
    printf("Money : %d\n", game->money);
    printf("A : ALU, price %d\n", type_to_price('A'));
    printf("R : RAM, price %d\n", type_to_price('R'));
    printf("F : Firewall, price %d\n", type_to_price('F'));
    printf("P : PMMU, price %d\n", type_to_price('P'));
    printf("C : CPU, price %d\n", type_to_price('C'));
    printf("V : View your defenses\n");
    printf("Q : no more\n");

    do {
        printf("Your choice ? \n");
        scanf(" %1c", &c);
//        c = getchar();
        fflush(stdin); // Important si l'utilisateur entre une phrase
        if (strchr("ARFPC", c) != NULL) {
            if (game->money - type_to_price(c) >= 0) {
                game->money = game->money - type_to_price(c);
                printf("Money left : %d\n", game->money);
                do {
                    taken = 0;
                    printf("Entrer la ligne (entre 1 et %d) et la position "
                           "(entre 1 et %d) de votre tourette.\n", LONGEUR_PL,
                           LARGEUR_PL - 1);
                    scanf(" %d %d", &line, &position);
                    if (in_plateau(line, position + 1) && game_tab[line -
                                                                   1][position - 1] != '.') {
                        printf("Une tourette est deja placee ici\n");
                        taken = 1;
                    }
                    fflush(stdin); // Important si l'utilisateur essaie de
                    // planter le programme
                } while (!(in_plateau(line, position + 1)) || taken);
                insererChip(&(game->chips), c, line, position);
                game_tab[line - 1][position - 1] = c;
            } else { printf("Pas assez d'argents\n"); }
        } else {
            if (c == 'V')
                affiche_game_tab(game_tab, game);
            else if (c != 'Q')
                printf("Mauvais saisi\n");
        }
    } while (c != 'Q');
}

int fin_jeu(Game *game) {
    Liste_Virus lst_v = game->virus;
    if (lst_v == NULL)
        return 1; // Pas de virus restant
    for (; lst_v != NULL; lst_v = lst_v->next) {
        if (lst_v->position <= 0)
            return -1; // Virus a atteint l'ordi
    }
    return 0; // Jeu n'est pas terminé
}


void
entree_plateau(Game *game, char game_tab[LONGEUR_PL][LARGEUR_PL]) {
    Liste_Virus lst_v = game->virus;
    for (; lst_v != NULL; lst_v = lst_v->next) {
        // Si jamais l'utilisateur place des defences très près de l'entrée
        // des virus et que ceux ci sont bloqués, il faut les faire entrer
        // même après leur tour, d'où lst_v->turn <= game->turn
        if (lst_v->turn <= game->turn && lst_v->position > LARGEUR_PL &&
            !(place_taken(game, lst_v->line, LARGEUR_PL))) {
            lst_v->position = LARGEUR_PL;
        }
    }
}


void deplacement_virus(Game *game) {
    Liste_Virus lst_v = game->virus;
    int i;
    for (; lst_v != NULL; lst_v = lst_v->next) {
        if (lst_v->position <= LARGEUR_PL) {
            for (i = 0; i < lst_v->speed; i++) {
                if (!(place_taken(game, lst_v->line, lst_v->position - 1))) {
                    lst_v->position -= 1;
                } else
                    break;
            }
        }
    }
}


void action_ALU(Game *game, Chips *chip) {
    Liste_Virus lst_v = game->virus;
    for (; lst_v != NULL; lst_v = lst_v->next) {
        if (chip->line == lst_v->line && lst_v->position <= LARGEUR_PL) {
            // Inf à LARGEUR_PL -> à virus dans le plateau
            lst_v->life -= 2;
            if (lst_v->life <= 0)
                supprimevirus(&(game->virus), lst_v);
            break;
        }
    }
}

void action_RAM(Game *game, Chips *chip) {
    Liste_Virus lst_v = game->virus;
    for (; lst_v != NULL; lst_v = lst_v->next) {
        if (chip->line == lst_v->line && lst_v->position <= LARGEUR_PL) {
            lst_v->speed = 1;
            break;
        }
    }
}

void action_PMMU(Game *game, Chips *chip) {
    Liste_Virus lst_v = game->virus;
    Liste_Virus tmp;
    while (lst_v != NULL) {
        if (chip->line == lst_v->line && lst_v->position <= LARGEUR_PL) {
            lst_v->life--;
            if (lst_v->life <= 0) {
                tmp = lst_v;
                lst_v = lst_v->next;
                supprimevirus(&(game->virus), tmp);
                continue;

            }
        }
        lst_v = lst_v->next;
    }
}


void action_CPU(Game *game, Chips *chip) {
    Liste_Virus lst_v = game->virus;
    Liste_Virus tmp;
    int atk_up = 0, atk_middle = 0, atk_down = 0;
    while (lst_v) {
        if (lst_v->position <= LARGEUR_PL) {
            if (lst_v->line == chip->line && !atk_middle) {
                atk_middle = 1;
                lst_v->life--;
            } else if (lst_v->line == chip->line - 1 && !atk_up) {
                atk_up = 1;
                lst_v->life--;
            } else if (lst_v->line == chip->line + 1 && !atk_down) {
                atk_down = 1;
                lst_v->life--;
            }
            if (lst_v->life <= 0) {
                tmp = lst_v;
                lst_v = lst_v->next;
                supprimevirus(&(game->virus), tmp);
                continue;
            }
            if (atk_up && atk_middle && atk_down)
                return;
        }
        lst_v = lst_v->next;
    }
}

void actions_chips(Game *game) {
    Liste_Chips lst_c = game->chips;
    for (; lst_c != NULL; lst_c = lst_c->next) {
        switch (lst_c->type_i) {
            case 1:
                action_ALU(game, lst_c);
                break;
            case 2:
                action_RAM(game, lst_c);
                break;
            case 3:
                break;
            case 4:
                action_PMMU(game, lst_c);
                break;
            case 5:
                action_CPU(game, lst_c);
                break;
            default:;
        }
    }
}

void attack_chip(Game *game, Virus *virus, int damage) {
    Liste_Chips lst_c = game->chips;
    for (; lst_c != NULL; lst_c = lst_c->next) {
        if ((virus->line == lst_c->line) &&
            (lst_c->position == virus->position - 1)) {
            lst_c->life -= damage;
            if (lst_c->life <= 0)
                supprimechip(&(game->chips), lst_c);
            break;
        }
    }
}


void action_FPE(Game *game, Virus *virus) {
    attack_chip(game, virus, 2);
}

void action_CM(Game *game, Virus *virus) {
    attack_chip(game, virus, 1);
}

void action_BS(Game *game, Virus *virus) {
    attack_chip(game, virus, 4);
}

void action_DDOS(Game *game, Virus *virus) {
    int range;
    attack_chip(game, virus, 1);
    range = 2;
    if (virus->next_line && virus->next_line->position <= virus->position -
                                                          range)
        virus->next_line->life++;
    if (virus->prev_line && virus->prev_line->position >= virus->position +
                                                          range)
        virus->prev_line->life++;
}

void action_SEGF(Game *game, Virus *virus) {
    int range;
    attack_chip(game, virus, 1);
    range = 2;
    if (virus->next_line && virus->next_line->position >= virus->position -
                                                          range) {
        virus->life++;
        virus->next_line->life--;
        if (virus->next_line->life <= 0)
            supprimevirus(&(game->virus), virus->prev_line);
    }

}

void actions_virus(Game *game) {
    Liste_Virus lst_v = game->virus;
    for (; lst_v != NULL; lst_v = lst_v->next) {
        if (1 <= lst_v->position && lst_v->position <= 24) {
            switch (lst_v->type_i) {
                case 1:
                    action_FPE(game, lst_v);
                    break;
                case 2:
                    action_CM(game, lst_v);
                    break;
                case 3:
                    action_BS(game, lst_v);
                    break;
                case 4:
                    action_DDOS(game, lst_v);
                    break;
                case 5:
                    action_SEGF(game, lst_v);
                    break;
            }
        }
    }
}

int main(int argc, char *argv[]) {
    Game chip_v_virus = {.virus = NULL, .chips = NULL, .turn = 1};
    int code_fin, maxturn = 0;
    initialise_Vlst_from_file(&chip_v_virus, argv[1], &maxturn);
    char tab_wave[LONGEUR_PL][maxturn];
    char game_tab[LONGEUR_PL][LARGEUR_PL];
    initialise_tab_wave(chip_v_virus.virus, maxturn, tab_wave);
    init_maj_game_tab(game_tab, &chip_v_virus);
    choix_et_selection(&chip_v_virus, maxturn, tab_wave, game_tab);

    printf("\e[1;1H\e[2J");

    while (!(code_fin = fin_jeu(&chip_v_virus))) {
        entree_plateau(&chip_v_virus, game_tab);

        init_maj_game_tab(game_tab, &chip_v_virus);
        affiche_game_tab(game_tab, &chip_v_virus);
        usleep(350000);
        printf("\e[1;1H\e[2J");

        actions_chips(&chip_v_virus);

        init_maj_game_tab(game_tab, &chip_v_virus);
        affiche_game_tab(game_tab, &chip_v_virus);
        usleep(350000);
        printf("\e[1;1H\e[2J");

        actions_virus(&chip_v_virus);

        init_maj_game_tab(game_tab, &chip_v_virus);
        affiche_game_tab(game_tab, &chip_v_virus);
        usleep(350000);
        printf("\e[1;1H\e[2J");

        deplacement_virus(&chip_v_virus);

        init_maj_game_tab(game_tab, &chip_v_virus);
        affiche_game_tab(game_tab, &chip_v_virus);
        usleep(350000);
        printf("\e[1;1H\e[2J");

        chip_v_virus.turn++;
    }

    affiche_game_tab(game_tab, &chip_v_virus);
    if (code_fin == 1)
        printf("\nBien joue, vous avez reussi a eradiquer tous les virus");
    else
        printf("\nOh no! les virus ont atteint votre ordinateur personnel ! il "
               "est maintenant infecte...");

    freeListChips(&(chip_v_virus.chips));
    freeListVirus(&(chip_v_virus.virus));

    return 0;
}
