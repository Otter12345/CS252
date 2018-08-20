/**
 * This file contains some helpful functions that you may wish to use in your server
 */

#include "misc.hh"

#include <unistd.h>
#include <sys/wait.h>

#include <algorithm>
#include <iostream>

/**
 * Trims all of the left whitespace off of a string
 */
std::string ltrim(std::string str) {
    str.erase(0, str.find_first_not_of(" "));
    return str;
}

/**
 * Trims all of the right whitespace off of a string
 */
std::string rtrim(std::string str) {
    str.erase(str.find_last_not_of(" ") + 1);
    return str;
}

/**
 * Trims all of the left and right whitespace off of a string
 */
std::string trim(std::string str) {
    return ltrim(rtrim(str));
}

/**
 * Returns the Content-Type value that should be used for a given filename
 */
std::string get_content_type(const std::string& filename) {
    std::string response;
    int pipe_fd[2];
    if (pipe(pipe_fd) == -1) {
        perror("get_content_type pipe error");
        exit(-1);
    }
    int pid = fork();

    if (pid == -1) {
        perror("get_content_type fork error");
        exit(-1);
    }
    if (pid == 0) {
        close(pipe_fd[0]);  // close read end
        dup2(pipe_fd[1], STDOUT_FILENO);
        dup2(pipe_fd[1], STDOUT_FILENO);
        close(pipe_fd[1]);

        execl("/usr/bin/file", "file", "-biE", filename.c_str(), NULL);
        perror("get_content_type execl error");
        exit(-1);
    } else {
        close(pipe_fd[1]);  // close write end

        char buf;
        while (read(pipe_fd[0], &buf, 1) > 0) {
            response += buf;
        }
        close(pipe_fd[0]);  // close read end
/*
        int status;
        if (waitpid(pid, &status, 0) == -1) {
            perror("get_content_type waitpid error");
            exit(-1);
        }
        if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
            std::cerr << "get_content_type returned nonzero status for " << filename << std::endl;
            response.clear();
        }
		*/
    }

    response.erase(std::remove_if(response.begin(), response.end(), isspace), response.end());
    return response;
}
