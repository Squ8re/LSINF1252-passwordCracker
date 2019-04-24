// Fichier qui regroupe le code utile a la lecture des options donnees en ligne de commande par l'utilisateur

#ifndef PARSING_HEADER
#define PARSING_HEADER

// Structure qui contient les differentes informations relatives aux options passees par l'utilisateur
// Mise dans le .h et pas dans le .c afin de la rendre "publique"
// cf. https://stackoverflow.com/questions/6316987/should-struct-definitions-go-in-h-or-c-file
typedef struct options {
		unsigned int n_threads; // Nombre de threads de *calcul* (= utilises pour appeler reversehash)
		char *out_file_name; // Nom du fichier de sortie pour les resultats du programme
		char **in_files_names;  // Noms des fichiers contenant les hashes
		unsigned int n_files;   // Nombre de fichiers d'input
		bool c_flag;            // true si l'option "-c" a ete utilisee
		bool o_flag;            // true si l'option "-o" a ete utilisee
		bool t_flag;            // true si l'option "-t" a ete utilisee
} options_t;

int parse_options(int argc, char **argv, options_t *user_options);

#endif
