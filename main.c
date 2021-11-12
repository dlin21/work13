#include <stdio.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>


struct pop_entry {
  int year;
  int pop;
  char boro[15];
};

long file_size(char * f){
  struct stat stats;
  stat(f, &stats);
  return stats.st_size;
}

void read_csv(char *csv){
  printf("reading nyc_pop.csv...\n");

  int file, err, i;
  int size = file_size(csv);
  char data[size];

  // open
  file = open(csv, O_RDONLY);
  if (file == -1){
    printf("%s\n", strerror(errno));
    return;
  }

  // read
  err = read(file, data, size);
  if (err == -1){
    printf("%s\n", strerror(errno));
    return;
  }
  
  // count lines
  int count = 0;
  for(i=0; i<size; i++){
    if(data[i]=='\n') count++;
  }

  // csv into struct
  int temp = 0;
  int j;
  struct pop_entry entry[5*count-5];
  char *boro[5] = {"Manhattan", "Brooklyn", "Queens", "Bronx", "Staten Island"};
  for(i=0; i<count-1; i++){
    int line[6];

    while(data[temp] != '\n') temp++;
    temp++;
    sscanf(data+temp, "%d, %d, %d, %d, %d, %d\n", &line[0], &line[1], &line[2], &line[3], &line[4], &line[5]);

    for(j=1; j<6; j++){
      // array of struct pop_entry
      entry[5*i+j-1].year = line[0];
      strcpy(entry[(5*i)+j-1].boro, boro[j-1]);
      entry[(5*i)+j-1].pop = line[j];
    }
  }

  // permissions
  int per = open("nyc_pop.data", O_CREAT, O_WRONLY, 0644);
  if(per == -1){
    printf("%s\n", strerror(errno));
    return;
  }
  err = write(per, entry, sizeof(entry));
  if(per == -1){
    printf("%s\n", strerror(errno));
    return;
  }

  printf("wrote %ld bytes to %s\n", file_size("nyc_pop.data"), csv);
}

void read_data(){
  int size = file_size("nyc_pop.data");
  int file = open("nyc_pop.data", O_RDONLY);
  // run -read_csv first
  if (file == -1){
    printf("%s", strerror(errno));
    return;
  }

  struct pop_entry *entry = malloc(file);
  int err = read(file, entry, size);
  if (err == -1){
    printf("%s\n", strerror(errno));
    return;
  }
  
  int i;
  int num_entry = size / sizeof(struct pop_entry);
  for(i=0; i<num_entry; i++) printf("%d\t year: %d\t boro: %s\t pop: %d\n", i, entry[i].year, entry[i].boro, entry[i].pop);
  free(entry);
}

void add_data(){
  int file = open("nyc_pop.data", O_WRONLY | O_APPEND);
  if(file == -1){
    printf("%s", strerror(errno));
    return;
  }

  // get input from command line
  char in[100];
  printf("Enter year boro pop: \n");
  fgets(in, sizeof(in), stdin);
  char boro[15];
  int yr;
  int population;
  sscanf(in, "%d %s %d", &yr, boro, &population);

  // new struct to store input
  struct pop_entry new;
  strncpy(new.boro, boro, 100);
  new.year = yr;
  new.pop = population;

  write(file, &new, sizeof(struct pop_entry));
  close(file);
}

void update_data(){
  read_data();
  int file = open("nyc_pop.data", O_RDWR);
  if(file == -1){
    printf("%s\n", strerror(errno));
    return;
  }

  struct stat *input = malloc(sizeof(struct stat));
  stat("nyc_pop.csv", input);
  int size = input->st_size;
  int ent = size / sizeof(struct pop_entry);

  int change;
  printf("entry you would like to update: \n");
  char update[100];
  fgets(update, sizeof(update), stdin);
  sscanf(update, "%d\n", &change);

  printf("Enter year boro pop: \n");
  struct pop_entry temp;
  char path[100];
  char boro[15];
  fgets(path, sizeof(path), stdin);
  sscanf(path, "%d %s %d\n", &temp.year, temp.boro, &temp.pop);

  lseek(file, change * sizeof(struct pop_entry), SEEK_SET);
  int err = write(file, &temp, sizeof(struct pop_entry));
  if(err == -1){
    printf("%s\n", strerror(errno));
    return;
  }
  printf("File update.\n");
}



int main(int argc, char *argv[]) {
  if (argv[1] == NULL) {
    printf("No commands given\n");
    return 0;
  } 
  else if (strcmp(argv[1], "-read_csv") == 0) read_csv("nyc_pop.csv");
  else if (strcmp(argv[1], "-read_data") == 0) read_data();
  else if (strcmp(argv[1], "-add_data") == 0) add_data();
  else if (strcmp(argv[1], "-update_data") == 0) update_data();
  else printf("Invalid command\n");
    
  return 0;
}