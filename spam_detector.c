#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <ctype.h>
#include <math.h>

#define PRAG 35

int count_emails(void)
{
	int nr = 0;

	DIR *data;
	struct dirent *email;

	data = opendir("./data/emails");
	if (!data) {
		fprintf(stderr, "Cannot open directory");
		exit(-1);
	}

	while ((email = readdir(data)) != NULL) {
		if (email->d_type == DT_REG)
			nr++;
	}
	closedir(data);

	return nr;
}

char *alloc_string(int size)
{
	char *str = (char *)malloc(size * sizeof(char));
	if (!str) {
		fprintf(stderr, "malloc failed");
		return NULL;
	}
	return str;
}

char **get_keywords(int *n_words)
{
	FILE *f = fopen("./data/keywords", "rt");
	if (!f) {
		fprintf(stderr, "cannot open file");
		exit(-1);
	}
	fscanf(f, "%d", n_words);

	char **keywords = (char **)malloc(*n_words * sizeof(char *));
	if (!keywords) {
		fprintf(stderr, "malloc failed");
		fclose(f);
		exit(-1);
	}
	for (int i = 0; i < *n_words; i++) {
		keywords[i] = alloc_string(10000);
		if (!keywords[i]) {
			for (int j = 0; j < i; j++)
				free(keywords[j]);
			free(keywords);
			exit(-1);
		}
	}

	for (int i = 0; i < *n_words; i++)
		fscanf(f, "%s", keywords[i]);

	fclose(f);
	return keywords;
}

int count_word(char *email, char *word)
{
	int freq = 0;
	char *name = alloc_string(100);
	strcpy(name, "./data/emails/");
	strcat(name, email);

	FILE *f = fopen(name, "rt");
	if (!f) {
		fprintf(stderr, "cannot open file");
		exit(-1);
	}

	char *line = alloc_string(10000);
	fgets(line, 10000, f);
	fgets(line, 10000, f);
	fgets(line, 10000, f);
	while (fgets(line, 10000, f)) {
		int sz = strlen(line);
		for (int i = 0; i < sz; i++)
			line[i] = tolower(line[i]);
		char *p = strstr(line, word);
		while (p) {
			freq++;
			p = strstr(p + 1, word);
		}
	}
	free(name);
	free(line);
	fclose(f);

	return freq;
}

void keyword_info(char *keyword, int n_emails)
{
	DIR *data;
	struct dirent *email;

	data = opendir("./data/emails");
	if (!data) {
		fprintf(stderr, "Cannot open directory");
		exit(-1);
	}

	FILE *stat = fopen("statistics.out", "a+t");
	if (!stat) {
		fprintf(stderr, "cannot open file");
		exit(-1);
	}

	double *freq = (double *)malloc(n_emails * sizeof(double));
	for (int i = 0; i < n_emails; i++)
		freq[i] = 0;

	int count = 0;
	while ((email = readdir(data)) != NULL) {
		if (email->d_type == DT_REG) {
			freq[count] = count_word(email->d_name, keyword);
			count++;
		}
	}

	double m_a = 0, dev = 0;
	for (int i = 0; i < n_emails; i++)
		m_a += freq[i];

	fprintf(stat, "%s ", keyword);
	fprintf(stat, "%d ", (int)m_a);

	m_a = 1.0 * m_a / n_emails;
	for (int i = 0; i < n_emails; i++)
		dev += 1.0 * (freq[i] - m_a) * (freq[i] - m_a) / n_emails;
	dev = sqrt(dev);
	fprintf(stat, "%.6lf\n", dev);

	closedir(data);
	fclose(stat);
	free(freq);
}

void get_size(double *bsize, double *avg_size)
{
	DIR *data;
	struct dirent *email;

	data = opendir("./data/emails");
	if (!data) {
		fprintf(stderr, "Cannot open directory");
		exit(-1);
	}
	while ((email = readdir(data)) != NULL) {
		if (email->d_type == DT_REG) {
			char *name = alloc_string(100);
			strcpy(name, "./data/emails/");
			strcat(name, email->d_name);

			FILE *f = fopen(name, "rt");
			if (!f) {
				fprintf(stderr, "cannot open file");
				exit(-1);
			}

			char *line = alloc_string(10000);
			fgets(line, 10000, f);
			fgets(line, 10000, f);
			fgets(line, 10000, f);
			while (fgets(line, 10000, f)) {
				int sz = strlen(line);
				*avg_size += sz;
				bsize[atoi(email->d_name)] += sz;
			}
			fclose(f);
			free(line);
			free(name);
		}
	}
	closedir(data);
}

double count_caps(char *email)
{
	FILE *f = fopen(email, "rt");
	if (!f) {
		fprintf(stderr, "cannot open file");
		exit(-1);
	}
	char *line = alloc_string(10000);
	fgets(line, 10000, f);
	fgets(line, 10000, f);
	fgets(line, 10000, f);

	double count = 0;
	while (fgets(line, 10000, f)) {
		int sz = strlen(line);
		for (int i = 0; i < sz; i++)
			if (isupper(line[i]))
				count++;
	}
	free(line);
	return count;
}

int check_spammer(char *email)
{
	FILE *f = fopen("./data/spammers", "rt");
	if (!f) {
		fprintf(stderr, "cannot open file");
		exit(-1);
	}

	FILE *mail = fopen(email, "rt");
	if (!f) {
		fprintf(stderr, "cannot open file");
		exit(-1);
	}
	char *line = alloc_string(10000);
	fgets(line, 10000, mail);
	fgets(line, 10000, mail);
	char *address = alloc_string(10000);
	do {
		fscanf(mail, "%s", address);
	} while (!strchr(address, '@'));
	if (address[strlen(address) - 1] == '>')
		address[strlen(address) - 1] = '\0';
	if (address[0] == '<') {
		char *tmp = address;
		tmp = alloc_string(10000);
		strcpy(tmp, address + 1);
		strcpy(address, tmp);
		free(tmp);
	}

	int n_spammers;
	char *spammer = alloc_string(10000);
	int score;
	fscanf(f, "%d", &n_spammers);
	for (int i = 0; i < n_spammers; i++) {
		fscanf(f, "%s%d", spammer, &score);
		if (strcmp(address, spammer) == 0) {
			free(address);
			free(spammer);
			free(line);
			fclose(f);
			fclose(mail);
			return score;
		}
	}

	free(address);
	free(spammer);
	free(line);
	fclose(f);
	fclose(mail);

	return 0;
}

char **get_more_keywords(int *add)
{
	FILE *f = fopen("./aditional_keywords", "rt");
	if (!f) {
		fprintf(stderr, "cannot open file");
		exit(-1);
	}
	fscanf(f, "%d", add);

	char **more_keywords = (char **)malloc(*add * sizeof(char *));
	if (!more_keywords) {
		fprintf(stderr, "malloc failed");
		fclose(f);
		exit(-1);
	}
	for (int i = 0; i < *add; i++) {
		more_keywords[i] = alloc_string(10000);
		if (!more_keywords[i]) {
			for (int j = 0; j < i; j++)
				free(more_keywords[j]);
			free(more_keywords);
			exit(-1);
		}
	}

	for (int i = 0; i < *add; i++)
		fscanf(f, "%s", more_keywords[i]);

	fclose(f);

	return more_keywords;
}

int main(void)
{
	int n_emails = count_emails();
	int n_words = 0;
	int add = 0;
	double *scores = (double *)malloc(n_emails * sizeof(double));
	double *bsize = (double *)malloc(n_emails * sizeof(double));
	char **keywords = get_keywords(&n_words);
	char **more_keywords = get_more_keywords(&add);
	double avg_size = 0;

	for (int i = 0; i < n_words; i++)
		keyword_info(keywords[i], n_emails);

	for (int i = 0; i < n_emails; i++) {
		bsize[i] = 0;
		scores[i] = 0;
	}

	get_size(bsize, &avg_size);
	avg_size = 1.0 * avg_size / n_emails;

	DIR *data;
	struct dirent *email;

	data = opendir("./data/emails");
	if (!data) {
		fprintf(stderr, "Cannot open directory");
		exit(-1);
	}

	while ((email = readdir(data)) != NULL) {
		if (email->d_type == DT_REG) {
			char *name = alloc_string(10000);
			strcpy(name, "./data/emails/");
			strcat(name, email->d_name);
			name[strlen(name)] = '\0';
			int number = atoi(email->d_name);
			for (int i = 0; i < n_words; i++)
				scores[number] += count_word(email->d_name, keywords[i]);
			for (int i = 0; i < add; i++)
				scores[number] += count_word(email->d_name, more_keywords[i]);
			scores[number] = 9 * scores[number]  * avg_size / bsize[number];

			double caps = count_caps(name);
			if (caps > bsize[number] / 16)
				scores[number] += 20;

			scores[number] += check_spammer(name);
			free(name);
		}
	}
	closedir(data);

	FILE *pred = fopen("prediction.out", "wt");
	if (!pred) {
		fprintf(stderr, "cannot open file");
		exit(-1);
	}

	for (int i = 0; i < n_emails; i++)
		if (scores[i] > PRAG)
			fprintf(pred, "1\n");
		else
			fprintf(pred, "0\n");
	//printf("%lf", scores[139]);
	fclose(pred);

	for (int i = 0; i < n_words; i++)
		free(keywords[i]);
	free(keywords);

	for (int i = 0; i < add; i++)
		free(more_keywords[i]);
	free(more_keywords);

	free(scores);
	free(bsize);
}

