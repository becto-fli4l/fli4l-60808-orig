/*****************************************************************************
 *  @file minunit.c
 *  Functions for unit testing.
 *
 *  This code is based on MinUnit, see:
 *  http://www.jera.com/techinfo/jtns/jtn002.html
 *
 *  Copyright (c) 2012-2016 The fli4l team
 *
 *  This file is part of fli4l.
 *
 *  fli4l is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  fli4l is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with fli4l.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Last Update: $Id$
 *****************************************************************************
 */

#include <testing/minunit.h>
#include <libmkfli4l/str.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>

int mu_num_tests_run = 0;
int mu_num_tests_failed = 0;
FILE *mu_stream = NULL;
static char *mu_report_dir = NULL;

/**
 * Describes the outcome of a single test case.
 */
struct mu_testcase {
    /**
     * The name of the test case.
     */
    char *name;
    /**
     * The result of the test case.
     */
    BOOL result;
    /**
     * The signal that caused the test case to terminate. If zero, the test
     * case terminted due to an assertion failure.
     */
    int signum;
    /**
     * The failure message of the test case (may be NULL).
     */
    char *msg;
    /**
     * The length of the failure message.
     */
    size_t msglen;
    /**
     * The captured stdout output of the test case (may be NULL).
     */
    char *out;
    /**
     * The length of the captured stdout output.
     */
    size_t outlen;
    /**
     * The captured stderr output of the test case (may be NULL).
     */
    char *err;
    /**
     * The length of the captured stderr output.
     */
    size_t errlen;
    /**
     * The duration of the test case in milliseconds.
     */
    unsigned long duration;
};

/**
 * Describes the outcome of a single test suite.
 */
struct mu_testsuite {
    /**
     * The name of the test suite.
     */
    char *name;
    /**
     * The number of tests run within the test suite.
     */
    int num_tests;
    /**
     * The number of tests failed due to an error within the test suite.
     */
    int num_errors;
    /**
     * The number of tests failed due to a failed assertion within the test
     * suite.
     */
    int num_failures;
    /**
     * The timepoint when the test suite has started its execution (resolution
     * up to a millisecond).
     */
    unsigned long start_tp;
    /**
     * The timepoint when the test suite has started its execution as a string
     * (resolution up to a second).
     */
    char *start_time;
    /**
     * Points to the tests of this suite that have already been executed.
     */
    struct mu_testcase *tests;
};

/**
 * Points to the current test suite.
 */
static struct mu_testsuite *mu_current_test_suite = NULL;

/**
 * Returns the current timepoint with millisecond resolution.
 *
 * @return
 *  A value describing the current timepoint with millisecond resolution.
 */
static unsigned long
mu_get_milliseconds(void)
{
    struct timeval tv;
#if defined(CLOCK_MONOTONIC)
    struct timespec tp;
    if (clock_gettime(CLOCK_MONOTONIC, &tp) == 0) {
        return tp.tv_sec * 1000uL + (tp.tv_nsec + 500000uL) / 1000000uL;
    }
#endif
    if (gettimeofday(&tv, NULL) == 0) {
        return tv.tv_sec * 1000uL + (tv.tv_usec + 500uL) / 1000uL;
    }
    else {
        return 0uL;
    }
}

/**
 * Converts a number of milliseconds in a human-readable duration.
 *
 * @param milliseconds
 *  The number of milliseconds elapsed.
 * @return
 *  A dynamically allocated string describing the number of milliseconds
 *  elapsed in a more readable form.
 */
static char *
mu_get_time_duration(unsigned long milliseconds)
{
    return safe_sprintf("%lu.%03lu", milliseconds / 1000, milliseconds % 1000);
}

/**
 * Converts a string into a form suitable for the contents of an XML attribute
 * node.
 *
 * @param msg
 *  The message to transform.
 * @return
 *  A dynamically allocated string containing the transformed message.
 */
static char *
mu_xmlify_attr(char const *msg)
{
    char *result;
    char const *p;
    char *q;
    size_t add_num = 0;
    static char const xml_lt[] = "&lt;";
    static char const xml_amp[] = "&amp;";
    static char const xml_quot[] = "&quot;";
    static char const xml_apos[] = "&apos;";

    p = msg;
    while (*p) {
        switch (*p) {
        case '<' :
            add_num += sizeof(xml_lt) - 2;
            break;
        case '&' :
            add_num += sizeof(xml_amp) - 2;
            break;
        case '"' :
            add_num += sizeof(xml_quot) - 2;
            break;
        case '\'' :
            add_num += sizeof(xml_apos) - 2;
            break;
        }
        ++p;
    }

    result = (char *) safe_malloc(strlen(msg) + add_num + 1);
    p = msg;
    q = result;
    while (*p) {
        switch (*p) {
        case '<' :
            strcpy(q, xml_lt);
            q += sizeof(xml_lt) - 1;
            break;
        case '&' :
            strcpy(q, xml_amp);
            q += sizeof(xml_amp) - 1;
            break;
        case '"' :
            strcpy(q, xml_quot);
            q += sizeof(xml_quot) - 1;
            break;
        case '\'' :
            strcpy(q, xml_apos);
            q += sizeof(xml_apos) - 1;
            break;
        default :
            *q++ = *p;
        }
        ++p;
    }

    *q = '\0';
    return result;
}

/**
 * Converts a string into a form suitable for the contents of an XML text node.
 *
 * @param msg
 *  The message to transform.
 * @return
 *  A dynamically allocated string containing the transformed message.
 */
static char *
mu_xmlify_text(char const *msg)
{
    static char const prefix[] = "<![CDATA[";
    static char const suffix[] = "]]>";

    size_t add_len = 0;
    char *result;
    char const *p = msg;
    char const *q;
    while ((p = strstr(p, suffix)) != NULL) {
        add_len += (sizeof(prefix) - 1) + (sizeof(suffix) - 1);
        ++p;
    }

    result = (char *) safe_malloc(
        strlen(msg) + (sizeof(prefix) - 1) + (sizeof(suffix) - 1) + add_len + 1
    );

    strcpy(result, prefix);
    p = msg;
    while ((q = strstr(p, suffix)) != NULL) {
        char *substr;
        q += 2;
        substr = substrsave(p, q);
        strcat(result, substr);
        free(substr);
        strcat(result, suffix);
        strcat(result, prefix);
        p = q;
    }
    strcat(result, p);
    strcat(result, suffix);
    return result;
}

/**
 * Begins a new test suite.
 *
 * @param name
 *  The name of the new testsuite.
 */
static void
mu_testsuite_new(char const *name)
{
    struct tm *tm;
    time_t t;

    mu_current_test_suite = (struct mu_testsuite *)
        safe_malloc(sizeof(struct mu_testsuite));
    mu_current_test_suite->name = strsave(name);
    mu_current_test_suite->num_tests = 0;
    mu_current_test_suite->num_errors = 0;
    mu_current_test_suite->num_failures = 0;
    mu_current_test_suite->tests = NULL;
    mu_current_test_suite->start_tp = mu_get_milliseconds();

    time(&t);
    tm = localtime(&t);
    mu_current_test_suite->start_time = safe_sprintf(
        "%04d-%02d-%02dT%02d:%02d:%02d",
        tm->tm_year + 1900,
        tm->tm_mon + 1,
        tm->tm_mday,
        tm->tm_hour,
        tm->tm_min,
        tm->tm_sec
    );
}

/**
 * Begins a new test case.
 *
 * @param name
 *  The name of the new test case.
 * @return
 *  A pointer to the mu_testcase structure associated with the new test case.
 */
static struct mu_testcase *
mu_testcase_new(char const *name) {
    struct mu_testcase *testcase;

    mu_current_test_suite->tests = (struct mu_testcase *)
        safe_realloc(mu_current_test_suite->tests,
            (mu_current_test_suite->num_tests + 1) * sizeof(struct mu_testcase));

    testcase = mu_current_test_suite->tests + mu_current_test_suite->num_tests;
    testcase->name = strsave(name);
    testcase->result = FALSE;
    testcase->signum = 0;
    testcase->msg = NULL;
    testcase->msglen = 0;
    testcase->out = NULL;
    testcase->outlen = 0;
    testcase->err = NULL;
    testcase->errlen = 0;
    testcase->duration = 0;

    ++mu_current_test_suite->num_tests;
    return testcase;
}

/**
 * Finishes the current test suite, if there is one. This includes writing the
 * test suite's log file.
 *
 * @param writeFile
 *  If TRUE, a log file is written.
 */
static void
mu_testsuite_finish(BOOL writeFile)
{
    FILE *f = NULL;
    char *name;
    char *suite_duration;
    struct mu_testcase *test;
    int i;

    if (!mu_current_test_suite) {
        return;
    }

    if (writeFile) {
        name = safe_sprintf(
            "%s/TEST-%s.xml",
            mu_report_dir ? mu_report_dir : ".",
            mu_current_test_suite->name
        );
        f = fopen(name, "w");
        if (f) {
            suite_duration = mu_get_time_duration(mu_get_milliseconds() - mu_current_test_suite->start_tp);
            fprintf(f, "<testsuite name=\"%s\" tests=\"%d\" errors=\"%d\" failures=\"%d\" host=\"localhost\" time=\"%s\" timestamp=\"%s\">\n",
                mu_current_test_suite->name,
                mu_current_test_suite->num_tests,
                mu_current_test_suite->num_errors,
                mu_current_test_suite->num_failures,
                suite_duration,
                mu_current_test_suite->start_time
            );
            free(suite_duration);
        }
        else {
            fprintf(stderr, "minunit error: cannot write test results to %s\n", name);
        }
        free(name);
    }
    free(mu_current_test_suite->start_time);

    test = mu_current_test_suite->tests;
    for (i = 0; i < mu_current_test_suite->num_tests; ++i, ++test) {
        if (f && writeFile) {
            char *duration = mu_get_time_duration(test->duration);
            fprintf(f, "<testcase classname=\"%s\" name=\"%s\" time=\"%s\">\n",
                mu_current_test_suite->name,
                test->name,
                duration
            );
            free(duration);

            if (!test->result) {
                if (test->signum == 0) {
                    char *msgcomplete = mu_xmlify_text(test->msg);
                    char *msg = mu_xmlify_attr(test->msg);
                    char *newline = strchr(msg, '\n');
                    if (newline) {
                        *newline = '\0';
                    }
                    fprintf(f, "<failure message=\"%s\" type=\"mu_assert\">%s</failure>\n",
                        msg,
                        msgcomplete
                    );
                    free(msgcomplete);
                    free(msg);
                }
                else {
                    char *msg = safe_sprintf("Terminated by signal %d.", test->signum);
                    fprintf(f, "<error message=\"%s\" type=\"mu_signal\">%s</error>\n",
                        msg,
                        msg
                    );
                    free(msg);
                }
            }

            if (test->out) {
                char *out = mu_xmlify_text(test->out);
                fprintf(f, "<system-out>%s</system-out>\n", out);
                free(out);
            }
            if (test->err) {
                char *err = mu_xmlify_text(test->err);
                fprintf(f, "<system-err>%s</system-err>\n", err);
                free(err);
            }

            fprintf(f, "</testcase>\n");
        }

        free(test->out);
        free(test->err);
        free(test->name);
        free(test->msg);
    }
    free(mu_current_test_suite->tests);

    if (f && writeFile) {
        fprintf(f, "</testsuite>\n");
        fclose(f);
    }

    free(mu_current_test_suite->name);
    free(mu_current_test_suite);
    mu_current_test_suite = NULL;
}

void
mu_schedule_test(char const *testsuite, char const *testcase, BOOL (*func)(void))
{
    int writepipes[2] = { -1, -1, };
    int readpipes[2] = { -1, -1, };
    int readerrpipes[2] = { -1, -1, };
    int readmsgpipes[2] = { -1, -1, };
    unsigned long start_tp;
    pid_t pid;
    int result;

    if (pipe(writepipes) < 0) {
        fprintf(stderr, "Error: Cannot create pipe for test process's stdin\n");
        result = 3;
    }
    else if (pipe(readpipes) < 0) {
        fprintf(stderr, "Error: Cannot create pipe for test process's stdout\n");
        close(writepipes[0]);
        close(writepipes[1]);
        result = 3;
    }
    else if (pipe(readerrpipes) < 0) {
        fprintf(stderr, "Error: Cannot create pipe for test process's stderr\n");
        close(writepipes[0]);
        close(writepipes[1]);
        close(readpipes[0]);
        close(readpipes[1]);
        result = 3;
    }
    else if (pipe(readmsgpipes) < 0) {
        fprintf(stderr, "Error: Cannot create pipe for test process's test message\n");
        close(writepipes[0]);
        close(writepipes[1]);
        close(readpipes[0]);
        close(readpipes[1]);
        close(readerrpipes[0]);
        close(readerrpipes[1]);
        result = 3;
    }
    else {
        start_tp = mu_get_milliseconds();
        pid = fork();
        if (pid < 0) {
            fprintf(stderr, "Error: Cannot fork test child\n");
            close(writepipes[0]);
            close(writepipes[1]);
            close(readpipes[0]);
            close(readpipes[1]);
            close(readerrpipes[0]);
            close(readerrpipes[1]);
            close(readmsgpipes[0]);
            close(readmsgpipes[1]);
            result = 3;
        }
        else if (pid > 0) {
            int exitcode;
            struct mu_testcase *test;

            close(writepipes[0]);
            close(writepipes[1]);
            close(readpipes[1]);
            close(readerrpipes[1]);
            close(readmsgpipes[1]);

            if (!mu_current_test_suite || strcmp(mu_current_test_suite->name, testsuite) != 0) {
                mu_testsuite_finish(TRUE);
                mu_testsuite_new(testsuite);
            }

            test = mu_testcase_new(testcase);

            while (TRUE) {
                fd_set rdfs;
                int ret;
                int maxfd = -1;

                FD_ZERO(&rdfs);
                if (readpipes[0] != -1) {
                    FD_SET(readpipes[0], &rdfs);
                    maxfd = readpipes[0] > maxfd ? readpipes[0] : maxfd;
                }
                if (readerrpipes[0] != -1) {
                    FD_SET(readerrpipes[0], &rdfs);
                    maxfd = readerrpipes[0] > maxfd ? readerrpipes[0] : maxfd;
                }
                if (readmsgpipes[0] != -1) {
                    FD_SET(readmsgpipes[0], &rdfs);
                    maxfd = readmsgpipes[0] > maxfd ? readmsgpipes[0] : maxfd;
                }

                if (maxfd == -1) {
                    break;
                }

                ret = select(maxfd + 1, &rdfs, NULL, NULL, NULL);
                if (ret == -1) {
                    break;
                }

                if (readpipes[0] != -1 && FD_ISSET(readpipes[0], &rdfs)) {
                    char buf[2048];
                    ssize_t bytes = read(readpipes[0], buf, sizeof buf);
                    if (bytes > 0) {
                        test->out = (char *) safe_realloc(test->out, test->outlen + bytes);
                        memcpy(test->out + test->outlen, buf, bytes);
                        test->outlen += bytes;
                    }
                    else {
                        close(readpipes[0]);
                        readpipes[0] = -1;
                    }
                }
                if (readerrpipes[0] != -1 && FD_ISSET(readerrpipes[0], &rdfs)) {
                    char buf[2048];
                    ssize_t bytes = read(readerrpipes[0], buf, sizeof buf);
                    if (bytes > 0) {
                        test->err = (char *) safe_realloc(test->err, test->errlen + bytes);
                        memcpy(test->err + test->errlen, buf, bytes);
                        test->errlen += bytes;
                    }
                    else {
                        close(readerrpipes[0]);
                        readerrpipes[0] = -1;
                    }
                }
                if (readmsgpipes[0] != -1 && FD_ISSET(readmsgpipes[0], &rdfs)) {
                    char buf[2048];
                    ssize_t bytes = read(readmsgpipes[0], buf, sizeof buf);
                    if (bytes > 0) {
                        test->msg = (char *) safe_realloc(test->msg, test->msglen + bytes);
                        memcpy(test->msg + test->msglen, buf, bytes);
                        test->msglen += bytes;
                    }
                    else {
                        close(readmsgpipes[0]);
                        readmsgpipes[0] = -1;
                    }
                }
            }

            waitpid(pid, &exitcode, 0);
            test->duration = mu_get_milliseconds() - start_tp;

            test->out = (char *) safe_realloc(test->out, test->outlen + 1);
            test->out[test->outlen] = '\0';
            test->err = (char *) safe_realloc(test->err, test->errlen + 1);
            test->err[test->errlen] = '\0';
            test->msg = (char *) safe_realloc(test->msg, test->msglen + 1);
            test->msg[test->msglen] = '\0';

            test->result = WIFEXITED(exitcode) && WEXITSTATUS(exitcode) == 0;
            if (!test->result) {
                if (WIFSIGNALED(exitcode)) {
                    test->signum = WTERMSIG(exitcode);
                    ++mu_current_test_suite->num_errors;
                }
                else if (test->msglen == 0) {
                    test->signum = SIGABRT;
                    ++mu_current_test_suite->num_errors;
                }
                else {
                    ++mu_current_test_suite->num_failures;
                }
            }

            result = test->result ? 0 : test->signum == 0 ? 1 : 2;
        }
        else {
            BOOL result;

            mu_testsuite_finish(FALSE);
            free(mu_report_dir);
            mu_report_dir = NULL;

            close(writepipes[1]);
            dup2(writepipes[0], 0);
            close(writepipes[0]);

            close(readpipes[0]);
            dup2(readpipes[1], 1);
            close(readpipes[1]);

            close(readerrpipes[0]);
            dup2(readerrpipes[1], 2);
            close(readerrpipes[1]);

            close(readmsgpipes[0]);
            mu_stream = fdopen(readmsgpipes[1], "w");
            result = (*func)();
            fclose(mu_stream);
            mu_stream = NULL;

            exit(result ? 0 : 1);
        }
    }

    switch (result) {
    case 0 :
        fprintf(stderr, ".");
        break;
    case 1 :
        fprintf(stderr, "F");
        ++mu_num_tests_failed;
        break;
    case 2 :
        fprintf(stderr, "E");
        ++mu_num_tests_failed;
        break;
    case 3 :
        fprintf(stderr, "!");
        ++mu_num_tests_failed;
        break;
    }

    ++mu_num_tests_run;
}

void
mu_test_finish(void)
{
    mu_testsuite_finish(TRUE);
    free(mu_report_dir);
    mu_report_dir = NULL;
}

void
mu_set_report_dir(char const *dir)
{
    free(mu_report_dir);
    mu_report_dir = strdup(dir);
}
