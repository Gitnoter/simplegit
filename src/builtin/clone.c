#include "common.h"
#include <git2.h>
#include <git2/clone.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

/* Shamelessly borrowed from http://stackoverflow.com/questions/3417837/ */
#ifdef UNUSED
#elif defined(__GNUC__)
# define UNUSED(x) UNUSED_ ## x __attribute__((unused))
#elif defined(__LCLINT__)
# define UNUSED(x) /*@unused@*/ x
#else
# define UNUSED(x) x
#endif

typedef struct progress_data {
	git_transfer_progress fetch_progress;
	size_t completed_steps;
	size_t total_steps;
	const char *path;
} progress_data;

static void print_progress(const progress_data *pd)
{
	int network_percent = (100*pd->fetch_progress.received_objects) / pd->fetch_progress.total_objects;
	int index_percent = (100*pd->fetch_progress.indexed_objects) / pd->fetch_progress.total_objects;
	int checkout_percent = pd->total_steps > 0
		? (100 * pd->completed_steps) / pd->total_steps
		: 0.f;
	int kbytes = pd->fetch_progress.received_bytes / 1024;

	fprintf(stderr, "net %3d%% (%4d kb, %5d/%5d)  /  idx %3d%% (%5d/%5d)  /  chk %3d%% (%4" PRIuZ "/%4" PRIuZ ") %s\r",
		   network_percent, kbytes,
		   pd->fetch_progress.received_objects, pd->fetch_progress.total_objects,
		   index_percent, pd->fetch_progress.indexed_objects, pd->fetch_progress.total_objects,
		   checkout_percent,
		   pd->completed_steps, pd->total_steps,
		   pd->path);
}

static int fetch_progress(const git_transfer_progress *stats, void *payload)
{
	progress_data *pd = (progress_data*)payload;
	pd->fetch_progress = *stats;
	print_progress(pd);
	return 0;
}
static void checkout_progress(const char *path, size_t cur, size_t tot, void *payload)
{
	progress_data *pd = (progress_data*)payload;
	pd->completed_steps = cur;
	pd->total_steps = tot;
	pd->path = path;
	print_progress(pd);
}

static int cred_acquire(git_cred **out,
		const char * UNUSED(url),
		const char * UNUSED(username_from_url),
		unsigned int UNUSED(allowed_types),
		void * UNUSED(payload))
{
	char username[128] = {0};
	char password[128] = {0};

	printf("Username: ");
	scanf("%s", username);

	/* Yup. Right there on your terminal. Careful where you copy/paste output. */
	printf("Password: ");
	scanf("%s", password);

	return git_cred_userpass_plaintext_new(out, username, password);
}

int do_clone(git_repository *repo, int argc, char **argv)
{
	progress_data pd = {{0}};
	git_repository *cloned_repo = NULL;
	git_clone_options clone_opts = GIT_CLONE_OPTIONS_INIT;
	git_checkout_options checkout_opts = GIT_CHECKOUT_OPTIONS_INIT;
	const char *url = argv[1];
	const char *path = argv[2];
	int error;

	(void)repo; // unused

	// Validate args
	if (argc < 3) {
		fprintf (stderr, "USAGE: %s <url> <path>\n", argv[0]);
		return -1;
	}

	// Set up options
	checkout_opts.checkout_strategy = GIT_CHECKOUT_SAFE_CREATE;
	checkout_opts.progress_cb = checkout_progress;
	checkout_opts.progress_payload = &pd;
	/* Do not bypass git-aware transport as otherwise, non-local URLs, e.g.
	 * beginning with https://, will be accessed as local files which on AmigaOS
	 * will provoke an Requester.
	 */
	clone_opts.local = GIT_CLONE_NO_LOCAL;
	clone_opts.checkout_opts = checkout_opts;
	clone_opts.remote_callbacks.transfer_progress = &fetch_progress;
	clone_opts.remote_callbacks.credentials = &cred_acquire;
	clone_opts.remote_callbacks.payload = &pd;

	// Do the clone
	error = git_clone(&cloned_repo, url, path, &clone_opts);
	printf("\n");
	if (error != 0) {
		const git_error *err = giterr_last();
		if (err) printf("ERROR %d: %s\n", err->klass, err->message);
		else printf("ERROR %d: no detailed info\n", error);
	}
	else if (cloned_repo) git_repository_free(cloned_repo);
	return error;
}
