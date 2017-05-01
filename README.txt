Project 2
Αγγελική Φελιμέγκα
1115201300192

Το Project 2 αποτελείται από τα εξής: jms_console.cpp jms_coord.cpp pool.cpp
το header file:pool.h 
και τέλος το bash script: jms_script.sh

jms_console.cpp: Στο πρόγραμμα αυτό διαβάζονται εντολές απο ένα αρχείο ή/kαι απο το standard input. Ανοίγονται τα απαραίτητα in,out pipes για επικοινωνια με τον coord και γράφονται οι διαβασμένες εντολές στο in και διαβάζονται τα αποτελέσματα απο το out. Ο τερματισμός γίνεται με την εντολη "exit".


jms_coord.cpp: Στο πρόγραμμα αυτό δημιουργούνται τα in,out pipes για επικοινωνία με το jms_console. Διαβάζονται οι εντολές απο το in και ανάλογα εκτελούνται οι απαραίτητες ενέργειες. Αφού εκτελεστούν τα αποτελέσματα γράφονται στο out και στελνονται έτσι στο jms_console.
Επίσης μέσω της εντολής submit job ο jms_coord δημιουργεί τα pool και τα απαραίτητα poolin, poolout pipes για επικοινωνια με αυτά.Κάθε φορά που γίνεται submit ελέγχει ποιο pool έχει κενή θέση να αναλάβει ένα job και του το "δίνει" διαφορετικά δημιουργεί ένα καινούργιο pool με fork.



pool.cpp - pool.h: Στα αρχεία αυτά υπάρχει η κλάση Pool με τις απαραίτητες συναρτήσεις για τον έλεγχο τον pool συμφωνα με την εκφώνηση.
Η κλάση περιέχει τα εξής μέλη: 	int jobs;   //Max number of jobs inside Pool
    				int current_jobs; //Current number of jobs inside Pool
			    	int *jobs_status; // 0 is for finished, 1 is for active, 2 is for suspended
			    	int *jobs_id;     //inique number 1,2,3,...
			    	pid_t *jobs_pid;
			    	pid_t pool_pid;
Αναλυτικότερα, η συνάρτηση Job_submit αναλαμβάνει κάθε φορά την εκτέλεση ενός job κάνοντας fork exec και αναθέτει τις κατάλληλες τιμές στα μέλη.
Οι suspendJob, resumeJob, Shutdown στένουν τα απαραίτητα σήματα στα processes-pool_jobs που έχουν δημιουργηθεί.


jms_script.sh: Στο σκριπτ αυτό αποθηκεύονται τα arguments σε μεταβλητές και ανάλογα εκτελούνται οι απαραίτητες ενέργεις σύμφωνα με την εκφώνηση.

Περιέχεται επίσης makefile.

Εντολές εκτέλεσης:

$make all
$./jms_coord -l <path> -n <jobs pool> -w <jms out> -r <jms in> &
$./jms_console -w <jms in> -r <jms out> -o <operations file>
$./jms script.sh -l <path> -c <command>
