#include <stdio.h>
#include <math.h>

#define STEPS 256
#define PER_LINE 16
#define RANGE 127

float t, u, v, w, d;
unsigned int i, j;
int svpwm[3][STEPS];

int main(int argc, char *argv[]) {
	t = 0;
	for (i = 0; i < STEPS; i++) {
		t += M_PI * 2 / STEPS;
		u = sin(t);
		v = sin(t + M_PI * 2 / 3);
		w = sin(t + M_PI * 4 / 3);
		d = u;
		if (fabs(v) < fabs(d)) d = v;
		if (fabs(w) < fabs(d)) d = w;
		d /= 2;
		svpwm[0][i] = round((u + d) * 2 / sqrt(3) * RANGE);
		svpwm[1][i] = round((v + d) * 2 / sqrt(3) * RANGE);
		svpwm[2][i] = round((w + d) * 2 / sqrt(3) * RANGE);
	}
	for (j = 0; j < 3; j++) {
		printf("const int8_t svpwm%c[] = {\n", 'U' + j, STEPS);
		for (i = 0; i < STEPS; i++) {
			if (i % PER_LINE == 0) printf("\t");
			printf("%i", svpwm[j][i]);
			if (i < STEPS - 1) {
				printf(",");
				if (i % PER_LINE == PER_LINE - 1)
					printf("\n");
				else
					printf(" ");
			}
		}
		printf("\n};\n\n");
	}

}
