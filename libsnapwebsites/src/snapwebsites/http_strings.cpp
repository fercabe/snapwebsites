// Snap Websites Server -- parse strings
// Copyright (C) 2013-2017  Made to Order Software Corp.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

#include "snapwebsites/http_strings.h"

#include "snapwebsites/log.h"

#include "snapwebsites/poison.h"


namespace snap
{
namespace http_strings
{




/** \brief The part_t constructor without parameters is for the vector.
 *
 * When initializing a vector, the class has to have a constructor with
 * no parameters. Unfortunate since we would prefer to not allow part_t
 * objects without a name, but mandatory (would have to test again once
 * we convert the class to only use the STL library.)
 */
WeightedHttpString::part_t::part_t()
    //: f_name("") -- auto-init
    //, f_level(DEFAULT_LEVEL) -- auto-init
{
}


/** \brief Create a named part_t.
 *
 * This function is used to create a valid part_t object.
 *
 * \param[in] name  The name of the part_t object.
 *
 * \sa get_name()
 */
WeightedHttpString::part_t::part_t(QString const & name)
    : f_name(name)
    //, f_level(DEFAULT_LEVEL)
{
}


/** \brief Retrieve the part_t name.
 *
 * The name of a part_t object cannot be changed once it was created.
 *
 * You may retrieve the name with this function, though.
 *
 * \bug
 * It is currently possible to create a part_t object without a name
 * so the class works with QVector.
 *
 * \return The name as passed in when create the part_t object.
 */
QString const & WeightedHttpString::part_t::get_name() const
{
    return f_name;
}


/** \brief Retrieve the level of this part_t object.
 *
 * This function retrieves the level of the part_t object. It is a floating
 * point value.
 *
 * The level is taken from the "q" parameter. For example, in:
 *
 * \code
 *      fr; q=0.3
 * \endcode
 *
 * the level is viewed as 0.3.
 *
 * \return The part_t object level.
 */
WeightedHttpString::part_t::level_t WeightedHttpString::part_t::get_level() const
{
    return f_level;
}


/** \brief Change the level of this part.
 *
 * This function saves the new \p level parameter in this part_t object.
 * Items without a level (q=<value>) parameter are assigned the special
 * value DEFAULT_LEVEL, which is 1.0.
 *
 * \bug
 * The function does not limit the level. It is expected to be defined
 * between 0.0 and 1.0, though.
 *
 * \param[in] level  The new part_t level.
 */
void WeightedHttpString::part_t::set_level(WeightedHttpString::part_t::level_t const level)
{
    f_level = level;
}


/** \brief Retrieve the value of a parameter.
 *
 * This function returns the value of a parameter given its name.
 *
 * If the parameter is not exist defined, then the function returns
 * an empty string. A parameter may exist and be set to the empty
 * string. There is no way to know at this point.
 *
 * \param[in] name  The name of the parameter to retrieve.
 *
 * \return The value of the parameter or "" if undefined.
 */
QString WeightedHttpString::part_t::get_parameter(QString const & name) const
{
    if(!f_param.contains(name))
    {
        return QString();
    }
    return f_param[name];
}


/** \brief Add a parameter.
 *
 * This function is used to add a parameter to the part_t object.
 *
 * A parameter has a name and a value.
 *
 * \param[in] name  The name of the parameter to add.
 * \param[in] value  The value of the parameter.
 */
void WeightedHttpString::part_t::add_parameter(QString const & name, QString const & value)
{
    f_param[name] = value;
}


/** \brief Convert one part back into a weighted HTTP string.
 *
 * This function builds one part of a weighted HTTP string. The string
 * will look something like:
 *
 * \code
 *      es; q=0.8
 * \endcode
 *
 * \return The part converted to one string.
 */
QString WeightedHttpString::part_t::to_string() const
{
    QString result;

    result = f_name;
    for(parameters_t::const_iterator it(f_param.begin());
                                     it != f_param.end();
                                     ++it)
    {
        QString p(it.key());
        if(!it.value().isEmpty())
        {
            p = QString("%1=%2").arg(p).arg(it.value());
        }
        result = QString("%1; %2").arg(result).arg(p);
    }

    return result;
}


/** \brief Operator used to sort elements.
 *
 * This operator overload is used by the different sort algorithms
 * that we can apply against this type. In most cases, it is a
 * std::stable_sort(),
 *
 * The function compares the level of the two part_t objects involved.
 *
 * Note that we sort from the largest to the smallest level. In other
 * words, if this part_t has level 1.0 and \p rhs has level 0.5, the
 * function returns true (i.e. 1.0 > 0.5).
 *
 * \param[in] rhs  The right hand side part_t object to compare against.
 *
 * \return true if this part_t is considered smaller than \p rhs.
 */
bool WeightedHttpString::part_t::operator < (part_t const & rhs) const
{
    return f_level > rhs.f_level;
}








/** \brief Create a new weighted HTTP string object.
 *
 * The constructor is most often passed a language string to be parsed
 * immediately. The string can be empty, though.
 *
 * This function calls the parse() function on the input string.
 *
 * \param[in] str  The list of weighted HTTP strings.
 */
WeightedHttpString::WeightedHttpString(QString const & str)
    //: f_str() -- auto-init
    //, f_parts() -- auto-init
{
    parse(str);
}


/** \brief Parse a weighted HTTP string.
 *
 * This function parses a weighted HTTP string with the standard format:
 *
 * \code
 *      start: strings
 *      strings: name opt_params
 *             | strings ',' name opt_params
 *      opt_params: <empty>
 *                | ';' param
 *                | opt_params ';' param
 *      param: param_name '=' param_value
 *      name: CHAR - [,;]
 *      param_name: CHAR [,;=]
 *      param_value: name
 * \endcode
 *
 * For example, the following defines a few language strings:
 *
 * \code
 *      fr;q=0.8,en;q=0.5,de;q=0.1
 * \endcode
 *
 * This ends up being parsed as:
 *
 * \li fr, level 0.8
 * \li en, level 0.5
 * \li de, level 0.1
 *
 * Note that the input can be in any order. The vector is returned in the
 * order it was read (first is most important if no levels at all were
 * specified).
 *
 * If you want to sort by level, make sure to retrieve the vector with
 * get_parts() and then sort it with sort_by_level().
 *
 * Remember that by default a part_t object uses the DEFAULT_LEVEL which
 * is 1.0. In other words, objects with no `q=...` parameter will likely
 * become first in the list.
 *
 * \code
 *      http_strings::WeightedHttpString language_country(locales);
 *      language_country.sort_by_level();
 * \endcode
 *
 * The "stable" is very important because if two strings have the same
 * level, then they have to stay in the order they were in the input
 * string.
 *
 * See reference:
 * https://www.w3.org/Protocols/rfc2616/rfc2616-sec14.html#sec14.4
 *
 * \note
 * The function may return false if errors were detected. However, it
 * will keep whatever strings were loaded so far.
 *
 * \todo
 * We may want to ameliorate the implementation to really limit all
 * the characters to what is clearly supported in HTTP/1.1 (Which
 * is the same in HTTP/2)
 *
 * \param[in] str  A weight HTTP string to parse.
 * \param[in] reset  Reset the existing weighted HTTP strings if true.
 *
 * \return true if no error were detected, false otherwise.
 */
bool WeightedHttpString::parse(QString const & str, bool reset)
{
    f_error_messages.clear();

    int pos(0);
    if(f_str.isEmpty() || reset)
    {
        f_parts.clear();
        f_str = str;
    }
    else
    {
        pos = f_str.length();
        f_str += ",";
        f_str += str;
    }

    QByteArray const utf8(f_str.toUtf8());
    char const * s(utf8.data() + pos);
    for(;;)
    {
        while(std::isspace(*s) || *s == ',')
        {
            ++s;
        }
        if(*s == '\0')
        {
            // reached the end of the string, we got a clean input
            //
            break;
        }
        char const * v(s);
        while(*s != '\0' && *s != ',' && *s != ';')
        {
            ++s;
        }

        // Note: we check the length of the resulting name, the
        //       RFC 2616 definition is:
        //
        //          language-tag  = primary-tag *( "-" subtag )
        //          primary-tag   = 1*8ALPHA
        //          subtag        = 1*8ALPHA
        //
        //       so the maximum size is 8 + 1 + 8 = 17 (1 to 8 characters,
        //       the dash, 1 to 8 characters) and the smallest is 1.
        //
        // TODO: add support for a value assigned to the first entry?
        //       (and thus mark it a part as well)
        //
        QString name(QString::fromUtf8(v, static_cast<int>(s - v)));
        name = name.simplified();
        if(name.isEmpty() || name.length() > 17)
        {
            // something is invalid, name is not defined (this can
            // happen if you just put a ';') or is too large
            //
            // XXX: should we signal the error in some way?
            //
            f_error_messages += "part name is empty or too long (limit is 17 characters.)\n";
            break;
        }
        // TODO: we want to check that `name` validity (i.e. 8ALPHA)
        //
        part_t part(name);

        // XXX: should we check whether another part with the same
        //      name already exists in the resulting vector?

        // read all the parameters, although we only keep
        // the 'q' parameter at this time
        //
        while(*s == ';')
        {
            ++s;
            // no need to skip spaces here because we later simplify the
            // parameter name
            //
            v = s;
            while(*s != '\0' && *s != ',' && *s != ';' && *s != '=')
            {
                ++s;
            }
            QString param_name(QString::fromUtf8(v, static_cast<int>(s - v)));
            param_name = param_name.simplified();

            // TODO: we want to check that `param_name` validity (i.e. `token`)
            //       all the following separators are not considered legal
            //       and also controls (< 0x20) and most certainly characters
            //       over 0x7E
            //
            //        separators     = "(" | ")" | "<" | ">" | "@"
            //                       | "," | ";" | ":" | "\" | <">
            //                       | "/" | "[" | "]" | "?" | "="
            //                       | "{" | "}" | SP | HT
            // See:
            // https://www.w3.org/Protocols/rfc2616/rfc2616-sec2.html#sec2.2
            //
            if(!param_name.isEmpty())
            {
                QString param_value;
                if(*s == '=')
                {
                    // TODO: the value may be a quoted string, the quotes
                    //       need to be removed and separators within the
                    //       quotes are to be accepted as part of the value
                    //
                    ++s;
                    v = s;
                    while(*s != '\0' && *s != ',' && *s != ';')
                    {
                        ++s;
                    }
                    param_value = QString::fromUtf8(v, static_cast<int>(s - v));
                    param_value = param_value.trimmed();
                }
                part.add_parameter(param_name, param_value);

                if(param_name == "q")
                {
                    bool ok(false);
                    WeightedHttpString::part_t::level_t const level(param_value.toFloat(&ok));
                    if(ok)
                    {
                        if(level >= 0.0)
                        {
                            part.set_level(level);
                        }
                        else
                        {
                            // The "quality" (q=...) parameter cannot be
                            // a negative number
                            //
                            f_error_messages += "the quality value (q=...) cannot be a negative number.\n";
                        }
                    }
                    else
                    {
                        // the "quality" (q=...) parameter is not a valid
                        // floating point value
                        //
                        f_error_messages += "the quality value (q=...) is not a valid floating point.\n";
                    }
                }
                // TODO add support for other parameters, "charset" is one of
                //      them in the Accept header which we want to support
            }
            else if(*s == '=')
            {
                // just ignore that entry...
                ++s;
                while(*s != '\0' && *s != ',' && *s != ';')
                {
                    ++s;
                }

                f_error_messages += "found a spurious equal sign in a weighted st.\n";
            }
        }
        f_parts.push_back(part);
    }

    if(!f_error_messages.isEmpty())
    {
        // in case the caller "forgets" to print errors...
        //
        SNAP_LOG_ERROR("parsing of \"")(str)("\" generated errors:\n")(f_error_messages);
    }

    return f_error_messages.isEmpty();
}


/** \brief Retrieve the level of the named parameter.
 *
 * This function searches for a part named \p name. If found, then its
 * level gets returned.
 *
 * A part with an unspecified level will have a level of DEFAULT_LEVEL
 * (which is 1.0f).
 *
 * If \p name is not found in the list of parts, this function returns
 * UNDEFINED_LEVEL (which is -1.0f).
 *
 * \param[in] name  The name of the part for which the level is requested.
 *
 * \return The part level or UNDEFINED_LEVEL.
 */
WeightedHttpString::part_t::level_t WeightedHttpString::get_level(QString const & name)
{
    const int max_parts(f_parts.size());
    for(int i(0); i < max_parts; ++i)
    {
        if(f_parts[i].get_name() == name)
        {
            return f_parts[i].get_level();
        }
    }
    return part_t::UNDEFINED_LEVEL();
}


/** \brief Use the weight (q=... values) to sort these HTTP strings.
 *
 * This function runs a stable sort against the weighted strings. This
 * is not called by default because some lists of strings are to
 * be kept sorted the way they are sent to us by the client.
 *
 * The function can be called multiple times, although, unless you
 * modify parts, there should be no need to do it more than once.
 */
void WeightedHttpString::sort_by_level()
{
    std::stable_sort(f_parts.begin(), f_parts.end());
}


/** \brief Convert all the parts to a full weighted HTTP string.
 *
 * This function converts all the parts of a weighted HTTP string
 * object to one string. The string representing each part is
 * generated using the part_t::to_string() function.
 *
 * \return The string representing this weighted HTTP string.
 */
QString WeightedHttpString::to_string() const
{
    QString result;
    int const max_parts(f_parts.size());
    for(int i(0); i < max_parts; ++i)
    {
        if(!result.isEmpty())
        {
            result += ", ";
        }
        result += f_parts[i].to_string();
    }
    return result;
}




} // namespace http_strings
} // namespace snap
// vim: ts=4 sw=4 et
