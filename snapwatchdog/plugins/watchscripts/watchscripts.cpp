// Snap Websites Server -- watchdog scripts
// Copyright (c) 2018  Made to Order Software Corp.  All Rights Reserved
//
// https://snapwebsites.org/
// contact@m2osw.com
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

// self
//
#include "watchscripts.h"

// snapwatchdog lib
//
#include "snapwatchdog.h"
#include "version.h"

// snapwebsites lib
//
#include <snapwebsites/snap_config.h>
#include <snapwebsites/email.h>
#include <snapwebsites/file_content.h>
#include <snapwebsites/glob_dir.h>
#include <snapwebsites/log.h>
#include <snapwebsites/not_used.h>
#include <snapwebsites/qdomhelpers.h>

// addr lib
//
#include <libaddr/addr.h>
#include <libaddr/iface.h>

// Qt lib
//
#include <QRegExp>



// last include
//
#include <snapwebsites/poison.h>


SNAP_PLUGIN_START(watchscripts, 1, 0)




/** \brief Get a fixed watchscripts plugin name.
 *
 * The watchscripts plugin makes use of different names. This function ensures
 * that you get the right spelling for a given name.
 *
 * \param[in] name  The name to retrieve.
 *
 * \return A pointer to the name.
 */
char const * get_name(name_t name)
{
    switch(name)
    {
    case name_t::SNAP_NAME_WATCHDOG_WATCHSCRIPTS_DEFAULT_LOG_SUBFOLDER:
        return "snapwatchdog-output";

    case name_t::SNAP_NAME_WATCHDOG_WATCHSCRIPTS_LOG_SUBFOLDER:
        return "log_subfolder";

    case name_t::SNAP_NAME_WATCHDOG_WATCHSCRIPTS_OUTPUT:
        return "watchdog_watchscripts_output";

    case name_t::SNAP_NAME_WATCHDOG_WATCHSCRIPTS_OUTPUT_DEFAULT:
        return "/var/lib/snapwebsites/snapwatchdog/script-files";

    case name_t::SNAP_NAME_WATCHDOG_WATCHSCRIPTS_PATH:
        return "watchdog_watchscripts_path";

    case name_t::SNAP_NAME_WATCHDOG_WATCHSCRIPTS_PATH_DEFAULT:
        return "/usr/share/snapwebsites/snapwatchdog/scripts";

    case name_t::SNAP_NAME_WATCHDOG_WATCHSCRIPTS_WATCH_SCRIPT_STARTER:
        return "watch_script_starter";

    case name_t::SNAP_NAME_WATCHDOG_WATCHSCRIPTS_WATCH_SCRIPT_STARTER_DEFAULT:
        return "/usr/bin/watch_script_starter";

    default:
        // invalid index
        throw snap_logic_exception("Invalid SNAP_NAME_WATCHDOG_WATCHSCRIPTS_...");

    }
    NOTREACHED();
}




/** \brief Initialize the watchscripts plugin.
 *
 * This function is used to initialize the watchscripts plugin object.
 */
watchscripts::watchscripts()
    //: f_snap(nullptr) -- auto-init
{
}


/** \brief Clean up the watchscripts plugin.
 *
 * Ensure the watchscripts object is clean before it is gone.
 */
watchscripts::~watchscripts()
{
}


/** \brief Get a pointer to the watchscripts plugin.
 *
 * This function returns an instance pointer to the watchscripts plugin.
 *
 * Note that you cannot assume that the pointer will be valid until the
 * bootstrap event is called.
 *
 * \return A pointer to the watchscripts plugin.
 */
watchscripts * watchscripts::instance()
{
    return g_plugin_watchscripts_factory.instance();
}


/** \brief Return the description of this plugin.
 *
 * This function returns the English description of this plugin.
 * The system presents that description when the user is offered to
 * install or uninstall a plugin on his website. Translation may be
 * available in the database.
 *
 * \return The description in a QString.
 */
QString watchscripts::description() const
{
    return "Check whether a set of watchscripts are running.";
}


/** \brief Return our dependencies.
 *
 * This function builds the list of plugins (by name) that are considered
 * dependencies (required by this plugin.)
 *
 * \return Our list of dependencies.
 */
QString watchscripts::dependencies() const
{
    return "|server|";
}


/** \brief Check whether updates are necessary.
 *
 * This function is ignored in the watchdog.
 *
 * \param[in] last_updated  The UTC Unix date when the website was last updated (in micro seconds).
 *
 * \return The UTC Unix date of the last update of this plugin.
 */
int64_t watchscripts::do_update(int64_t last_updated)
{
    NOTUSED(last_updated);
    SNAP_PLUGIN_UPDATE_INIT();
    // no updating in watchdog
    SNAP_PLUGIN_UPDATE_EXIT();
}


/** \brief Initialize watchscripts.
 *
 * This function terminates the initialization of the watchscripts plugin
 * by registering for various events.
 *
 * \param[in] snap  The child handling this request.
 */
void watchscripts::bootstrap(snap_child * snap)
{
    f_snap = snap;

    SNAP_LISTEN(watchscripts, "server", watchdog_server, process_watch, _1);

    f_watch_script_starter = [&]()
        {
            QString const starter(f_snap->get_server_parameter(get_name(name_t::SNAP_NAME_WATCHDOG_WATCHSCRIPTS_WATCH_SCRIPT_STARTER)));
            if(starter.isEmpty())
            {
                return QString::fromUtf8(get_name(name_t::SNAP_NAME_WATCHDOG_WATCHSCRIPTS_WATCH_SCRIPT_STARTER_DEFAULT));
            }
            return starter;
        }();
    setenv("WATCHDOG_WATCHSCRIPTS_LOG_PATH", f_log_path.toUtf8().data(), 1);

    // setup a variable that our scripts can use to save data as they
    // see fit; especially, many scripts need to remember what they've
    // done before or maybe they don't want to run too often and use a
    // file to know when to run again
    //
    QString const scripts_output([&]()
        {
            QString const filename(f_snap->get_server_parameter(get_name(name_t::SNAP_NAME_WATCHDOG_WATCHSCRIPTS_OUTPUT)));
            if(filename.isEmpty())
            {
                return QString::fromUtf8(get_name(name_t::SNAP_NAME_WATCHDOG_WATCHSCRIPTS_OUTPUT_DEFAULT));
            }
            return filename;
        }());
    setenv("WATCHDOG_WATCHSCRIPTS_OUTPUT", scripts_output.toUtf8().data(), 1);

    f_log_path = [&]()
        {
            QString const path(f_snap->get_server_parameter(get_name(watchdog::name_t::SNAP_NAME_WATCHDOG_LOG_PATH)));
            if(path.isEmpty())
            {
                return QString::fromUtf8(get_name(watchdog::name_t::SNAP_NAME_WATCHDOG_DEFAULT_LOG_PATH));
            }
            return path;
        }();
    setenv("WATCHDOG_WATCHSCRIPTS_LOG_PATH", f_log_path.toUtf8().data(), 1);

    f_log_subfolder = [&]()
        {
            QString const folder(f_snap->get_server_parameter(get_name(name_t::SNAP_NAME_WATCHDOG_WATCHSCRIPTS_LOG_SUBFOLDER)));
            if(folder.isEmpty())
            {
                return QString::fromUtf8(get_name(name_t::SNAP_NAME_WATCHDOG_WATCHSCRIPTS_DEFAULT_LOG_SUBFOLDER));
            }
            return folder;
        }();
    setenv("WATCHDOG_WATCHSCRIPTS_LOG_SUBFOLDER", f_log_subfolder.toUtf8().data(), 1);

    f_scripts_output_log = f_log_path + "/" + f_log_subfolder + "/snapwatchdog-scripts.log";
    f_scripts_error_log = f_log_path + "/" + f_log_subfolder + "/snapwatchdog-scripts-errors.log";
}


/** \brief Process this watchdog data.
 *
 * This function runs this watchdog.
 *
 * The process is to go through all the script in the snapwatchdog directory
 * and run them. If they exit with 2, then they detected a problem and we
 * send an email to the administrator. If they exit with 1, the script is
 * bogus and we send an email to the administrator. If they exit with 0,
 * no problem was discovered yet.
 *
 * The scripts are standard shell scripts. The snapwatchdog environment
 * offers additional shell commands, though, to ease certain things that
 * are otherwise very complicated.
 *
 * The results are also saved in the `doc` XML data.
 *
 * \param[in] doc  The document.
 */
void watchscripts::on_process_watch(QDomDocument doc)
{
    SNAP_LOG_TRACE("watchscripts::on_process_watch(): processing");

    QString const scripts_path([&]()
        {
            QString const path(f_snap->get_server_parameter(get_name(name_t::SNAP_NAME_WATCHDOG_WATCHSCRIPTS_PATH)));
            if(path.isEmpty())
            {
                return QString::fromUtf8(get_name(name_t::SNAP_NAME_WATCHDOG_WATCHSCRIPTS_PATH_DEFAULT));
            }
            return path;
        }());

    NOTUSED(doc);
    //QDomElement parent(snap_dom::create_element(doc, "watchdog"));
    //QDomElement w(snap_dom::create_element(parent, "watchscripts"));

    glob_dir const script_filenames(scripts_path + "/*", GLOB_ERR | GLOB_NOSORT | GLOB_NOESCAPE);

    // allow for failures, admins are responsible for making sure it will
    // work as expected
    //
    f_output_file.reset(new QFile(f_scripts_output_log));
    if(f_output_file != nullptr
    && !f_output_file->open(QIODevice::Append))
    {
        f_output_file.reset();
    }
    f_error_file.reset(new QFile(f_scripts_error_log));
    if(f_error_file != nullptr
    && !f_error_file->open(QIODevice::Append))
    {
        f_error_file.reset();
    }

    f_email.clear();

    script_filenames.enumerate_glob(std::bind(&watchscripts::process_script, this, std::placeholders::_1));

    if(!f_email.isEmpty())
    {
        // we got email data, send it
        //
        QString const from_email(f_snap->get_server_parameter(get_name(watchdog::name_t::SNAP_NAME_WATCHDOG_FROM_EMAIL)));
        QString const destination_email(f_snap->get_server_parameter(get_name(watchdog::name_t::SNAP_NAME_WATCHDOG_ADMINISTRATOR_EMAIL)));

        if(!from_email.isEmpty()
        && !destination_email.isEmpty())
        {
            email e;

            // set "From: ..." header
            //
            e.set_from(from_email);

            // set "To: ..." header
            //
            e.set_to(destination_email);

            // mark priority as Urgent
            //
            e.set_priority(email::priority_t::EMAIL_PRIORITY_URGENT);

            // set the subject
            //
            e.set_subject("Snap Watchdog Report: one or more watchdog scripts failed.");

            // prevent blacklisting
            // (since we won't run the validation, it's not necessary)
            //e.add_parameter(sendmail::get_name(sendmail::name_t::SNAP_NAME_SENDMAIL_BYPASS_BLACKLIST), "true");

            // add the email subject and body using a page
            email::attachment a;
            a.set_data(f_email.toUtf8(), "text/plain");
            e.set_body_attachment(a);

            // send the email
            //
            if(!e.send())
            {
                SNAP_LOG_ERROR("could not properly send the watchscript resulting email.");
            }
        }
        else
        {
            SNAP_LOG_ERROR("watchscripts cannot send an email without having 'from_email' and 'administrator_email' defined in your snapwatchdog.conf file.");
        }
    }
}


void watchscripts::process_script(QString script_filename)
{
    // setup the variable used while running a script
    //
    f_new_output_script = true;
    f_new_error_script = true;
    f_last_output_byte = '\n'; // whatever works in here, but I think this '\n' makes it clearer

    f_output.clear();
    f_error.clear();
    f_script_filename = script_filename;
    f_start_date = time(nullptr);

    // run the script
    //
    process p("watchscript");
    p.set_mode(process::mode_t::PROCESS_MODE_INOUTERR);
    p.set_command(f_watch_script_starter);  // scripts should not have the 'x' set so we have to use /bin/sh to start them
    p.add_argument(script_filename);
    p.set_output_callback(this);
    int const exit_code(p.run());

    // if we output some data and it did not ned with \n then add it now
    //
    if(!f_new_output_script
    && f_last_output_byte != '\n'
    && f_output_file != nullptr)
    {
        f_output_file->write("\n", 1);
        f_output += "\n";
    }
    if(!f_new_error_script
    && f_last_error_byte != '\n'
    && f_error_file != nullptr)
    {
        f_error_file->write("\n", 1);
        f_error += "\n";
    }

    if(!f_error.isEmpty()
    && exit_code == 0)
    {
        SNAP_LOG_WARNING("we got errors but the process exit code is 0");
    }

    // if we received some output, email it to the administrator
    // if we also had a failing script
    //
    if(exit_code != 0
    && !f_output.isEmpty())
    {
        // we do not want to send 20 different emails so instead we
        // generate a digest of all the output and then send that
        // to the admins once we are done running all the scripts.
        //
        // TODO: we need to cut the data if too large (we need to keep
        //       track of what we already added to f_email)
        //
        f_email += f_output;
    }
    if(exit_code != 0
    && !f_error.isEmpty())
    {
        // we do not want to send 20 different emails so instead we
        // generate a digest of all the output and then send that
        // to the admins once we are done running all the scripts.
        //
        // TODO: we need to cut the data if too large (we need to keep
        //       track of what we already added to f_email)
        //
        f_email += f_error;
    }
}


bool watchscripts::output_available(process * p, QByteArray const & output)
{
    NOTUSED(p);

    // ignore if empty (it should not happen but our code depends on it.)
    //
    if(output.isEmpty())
    {
        return true;
    }

    // generate a line to seperate each script entry
    //
    QString header;
    if(f_new_output_script)
    {
        header = QString("----------------------------------------------------------------------\n"
                         "Snap! Watchdog Version: " SNAPWATCHDOG_VERSION_STRING "\n"
                         "Date: %1\n"
                         "Script: %2\n")
                                .arg(format_date(f_start_date))
                                .arg(f_script_filename).toUtf8();

        snap::file_content hostname("/etc/hostname");
        if(hostname.read_all())
        {
            header += "Hostname: ";
            header += QString::fromUtf8(hostname.get_content().c_str()).trimmed();
            header += "\n";
        }

        // if we have a properly installed snapcommunicator use that IP
        //
        snap::snap_config config("snapcommunicator");
        QString const my_ip(config["my_address"]);
        if(!my_ip.isEmpty())
        {
            header += "IP Address: ";
            header += my_ip;
            header += "\n";
        }
        else
        {
            // no snapcommunicator defined "my_address", then show
            // all the IPs on this computer
            //
            addr::iface::vector_t const ips(addr::iface::get_local_addresses());
            if(!ips.empty())
            {
                header += "IP Addresses: ";
                QString sep;
                for(auto const & i : ips)
                {
                    header += sep;
                    header += i.get_address().to_ipv4or6_string(addr::addr::string_ip_t::STRING_IP_BRACKETS).c_str();
                    sep = ", ";
                }
                header += "\n";
            }
        }

        // we can immediately add it to the output buffer
        //
        f_output += header;

    }

    // add an empty line between the header or previous output
    // and the next output block
    //
    f_output += "\nMessage:\n";

    f_output += QString::fromUtf8(output);

    // if there is an output file, write that output data to it
    //
    if(f_output_file != nullptr)
    {
        // first write for this script? then write its name
        //
        if(f_new_output_script)
        {
            f_output_file->write(header.toUtf8());
        }
        f_output_file->write(output);

        // save the last byte so we know whether we had a "\n"
        //
        f_last_output_byte = output.at(output.length() - 1);
    }

    f_new_output_script = false;

    return true;
}


bool watchscripts::error_available(process * p, QByteArray const & error)
{
    NOTUSED(p);

    // ignore if empty (it should not happen but our code depends on it.)
    //
    if(error.isEmpty())
    {
        return true;
    }

    // generate a line to seperate each script entry
    //
    QString header;
    if(f_new_error_script)
    {
        // TODO: use a similar header as in the output_available()
        //
        header = QString("%1 ---------------------------------------- %2\n")
                                .arg(format_date(f_start_date))
                                .arg(f_script_filename).toUtf8();

        // we can immediately add it to the error buffer
        //
        f_error += header;
    }
    f_error += QString::fromUtf8(error);

    // if there is an error output file, write that error data to it
    //
    if(f_error_file != nullptr)
    {
        // first write for this script? then write its name
        //
        if(f_new_error_script)
        {
            f_error_file->write(header.toUtf8());
        }
        f_error_file->write(error);

        // save the last byte so we know whether we had a "\n"
        //
        f_last_error_byte = error.at(error.length() - 1);
    }

    f_new_error_script = false;

    return true;
}


QString watchscripts::format_date(time_t const t)
{
    struct tm f;
    gmtime_r(&t, &f);
    char date[32]; // YYYY/MM/DD HH:MM:SS (19 characters)
    strftime(date, sizeof(date) - 1, "%D %T", &f);
    return QString::fromUtf8(date);
}


SNAP_PLUGIN_END()

// vim: ts=4 sw=4 et
