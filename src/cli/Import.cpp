/*
 *  Copyright (C) 2019 KeePassXC Team <team@keepassxc.org>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 or (at your option)
 *  version 3 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <cstdlib>
#include <stdio.h>

#include <QFileInfo>
#include <QString>
#include <QTextStream>

#include "Import.h"

#include "cli/TextStream.h"
#include "cli/Utils.h"
#include "core/Database.h"

#include "keys/CompositeKey.h"
#include "keys/Key.h"

/**
 * Create a database file from an XML export of another database.
 * A password can be specified to encrypt the database.
 * If none is specified the function will fail.
 *
 * If the database is being saved in a non existant directory, the
 * function will fail.
 *
 * @return EXIT_SUCCESS on success, or EXIT_FAILURE on failure
 */
Import::Import()
{
    name = QString("import");
    description = QObject::tr("Import the contents of an XML database.");
    positionalArguments.append({QString("xml"), QObject::tr("Path of the XML database export."), QString("")});
    positionalArguments.append({QString("database"), QObject::tr("Path of the new database."), QString("")});
}

int Import::execute(const QStringList& arguments)
{
    TextStream outputTextStream(Utils::STDOUT, QIODevice::WriteOnly);
    TextStream errorTextStream(Utils::STDERR, QIODevice::WriteOnly);

    QSharedPointer<QCommandLineParser> parser = getCommandLineParser(arguments);
    if (parser.isNull()) {
        return EXIT_FAILURE;
    }

    const QStringList args = parser->positionalArguments();
    const QString xmlExportPath = args.at(0);
    const QString dbPath = args.at(1);

    if (QFileInfo::exists(dbPath)) {
        errorTextStream << QObject::tr("File %1 already exists.").arg(dbPath) << endl;
        return EXIT_FAILURE;
    }

    auto key = QSharedPointer<CompositeKey>::create();

    auto password = getPasswordFromStdin();
    if (!password.isNull()) {
        key->addKey(password);
    }

    if (key->isEmpty()) {
        errorTextStream << QObject::tr("No key is set. Aborting database creation.") << endl;
        return EXIT_FAILURE;
    }

    QString errorMessage;
    Database db;
    db.setKey(key);

    if (!db.import(xmlExportPath, &errorMessage)) {
        errorTextStream << QObject::tr("Unable to import XML database export %1").arg(errorMessage) << endl;
        return EXIT_FAILURE;
    }

    if (!db.save(dbPath, &errorMessage, true, false)) {
        errorTextStream << QObject::tr("Failed to save the database: %1.").arg(errorMessage) << endl;
        return EXIT_FAILURE;
    }

    outputTextStream << QObject::tr("Successfully imported database.") << endl;
    return EXIT_SUCCESS;
}

/**
 * Read optional password from stdin.
 *
 * @return Pointer to the PasswordKey or null if passwordkey is skipped
 *         by user
 */
QSharedPointer<PasswordKey> Import::getPasswordFromStdin()
{
    QSharedPointer<PasswordKey> passwordKey;
    QTextStream out(Utils::STDOUT, QIODevice::WriteOnly);

    out << QObject::tr("Insert password to encrypt database (Press enter to leave blank): ");
    out.flush();
    QString password = Utils::getPassword();

    if (!password.isEmpty()) {
        passwordKey = QSharedPointer<PasswordKey>(new PasswordKey(password));
    }

    return passwordKey;
}
