/*
  header file with common definitions for winlibs tools
*/

#ifndef INCLUDED_FIND_LOG4J_COMMON_H
#define INCLUDED_FIND_LOG4J_COMMON_H

/*! \brief version number constants
 * \name   FIND_LOG4J_VERSION_*
 * \{
 */
/*! \brief major version number */
#define FIND_LOG4J_VERSION_MAJOR 0
/*! \brief minor version number */
#define FIND_LOG4J_VERSION_MINOR 0
/*! \brief micro version number */
#define FIND_LOG4J_VERSION_MICRO 5
/*! @} */

/*! \brief packed version number */
#define FIND_LOG4J_VERSION (FIND_LOG4J_VERSION_MAJOR * 0x01000000 + FIND_LOG4J_VERSION_MINOR * 0x00010000 + FIND_LOG4J_VERSION_MICRO * 0x00000100)

/*! \cond PRIVATE */
#define FIND_LOG4J_VERSION_STRINGIZE_(major, minor, micro) #major"."#minor"."#micro
#define FIND_LOG4J_VERSION_STRINGIZE(major, minor, micro) FIND_LOG4J_VERSION_STRINGIZE_(major, minor, micro)
/*! \endcond */

/*! \brief string with dotted version number \hideinitializer */
#define FIND_LOG4J_VERSION_STRING FIND_LOG4J_VERSION_STRINGIZE(FIND_LOG4J_VERSION_MAJOR, FIND_LOG4J_VERSION_MINOR, FIND_LOG4J_VERSION_MICRO)

/*! \brief project license type \hideinitializer */
#define FIND_LOG4J_LICENSE "MIT"

/*! \brief project credits \hideinitializer */
#define FIND_LOG4J_CREDITS "Brecht Sanders (2021)"

#endif //INCLUDED_FIND_LOG4J_COMMON_H
