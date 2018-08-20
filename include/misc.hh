#ifndef INCLUDE_MISC_HH_
#define INCLUDE_MISC_HH_

#include <string>

/**
 * Trims all the left whitespace off of a string
 */
std::string ltrim(std::string str);

/**
 * Trims all the right whitespace off of a string
 */
std::string rtrim(std::string str);

/**
 * Trims all the left and right whitespace off of a string
 */
std::string trim(std::string str);

/**
 * \brief Gets a valid Content-Type string for the file specified by filename
 * 
 * \param filename
 *        The name of the file to query.
 * \return
 *        A valid Content-Type string, if possible. If an error occurred or the MIME 
 *        type couldn't be determined, then an empty string is returned
 */
std::string get_content_type(const std::string& filename);

#endif  // INCLUDE_MISC_HH_
