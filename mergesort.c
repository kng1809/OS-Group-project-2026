/**
 * This file implements parallel mergesort.
 */

#include <stdio.h>
#include <string.h> /* for memcpy */
#include <stdlib.h> /* for malloc */
#include "mergesort.h"

/* this function will be called by mergesort() and also by parallel_mergesort(). */
void merge(int leftstart, int leftend, int rightstart, int rightend){
	// Merge two already-sorted adjacent subarrays.
	int i = leftstart;
	int j = rightstart;
	int k = leftstart;

	// Compare elements and copy the smaller value into B.
	while (i <= leftend && j <= rightend) {
		if (A[i] <= A[j]) {
			B[k] = A[i];
			i++;
		} else {
			B[k] = A[j];
			j++;
		}
		k++;
	}

	// Copy any remaining elements from either subarray.
	while (i <= leftend) {
		B[k] = A[i];
		i++;
		k++;
	}

	while (j <= rightend) {
		B[k] = A[j];
		j++;
		k++;
	}

	// Copy the merged range back into A.
	memcpy(A + leftstart, B + leftstart, (rightend - leftstart + 1) * sizeof(int));

}

/* this function will be called by parallel_mergesort() as its base case. */
void my_mergesort(int left, int right){
	// Recurse only when the range contains more than one element.
	if (left < right) {
		int mid = left + (right-left)/2;
		my_mergesort(left, mid);
		my_mergesort(mid+1, right);
		merge(left, mid, mid+1, right);
	}
}

// this function will be called by the testing program.
void * parallel_mergesort(void *arg){
	struct argument *args = (struct argument *)arg;

	int left = args->left;
	int right = args->right;
	int level = args->level;

	// base case
	if (left >= right) {
		return NULL;
	}

	// Stop creating threads at the cutoff and finish this range serially.
	if (level >= cutoff) {
		my_mergesort(left, right);
		return NULL;
	}

	int mid = left + (right - left) / 2;

	struct argument *leftArgs = buildArgs(left, mid, level + 1);

	struct argument *rightArgs = buildArgs(mid + 1, right, level + 1);

	if (leftArgs == NULL || rightArgs == NULL) {
		free(leftArgs);
		free(rightArgs);

		my_mergesort(left, right);
		return NULL;
	}

	// Create child threads for the two halves.
	pthread_t t1, t2;

	// pthread_create stores each new thread ID in t1 or t2.
	int t1_created = pthread_create(&t1, NULL, parallel_mergesort, leftArgs);
	int t2_created =  pthread_create(&t2, NULL, parallel_mergesort, rightArgs);
	// pthread_create returns 0 on success and non-zero on failure.

	// Wait for each successfully-created child; otherwise sort that half locally.
	if (t1_created == 0) {
		pthread_join(t1, NULL);
	} else {
		my_mergesort(left, mid);
	}

	if (t2_created == 0) {
		pthread_join(t2, NULL);
	} else {
		my_mergesort(mid + 1, right);
	}

	merge(left, mid, mid + 1, right);

	free(leftArgs);
	free(rightArgs);

	return NULL;
}

// we build the argument for the parallel_mergesort function.
struct argument * buildArgs(int left, int right, int level){
	struct argument *arg = malloc(sizeof(struct argument));

	// check if malloc failed
	if (arg == NULL) {
		fprintf(stderr, "Error: malloc failed in buildArgs\n");
		return NULL;
	}

	// set the values of the argument struct
	arg->left = left;
	arg->right = right;
	arg->level = level;
	return arg;
}

