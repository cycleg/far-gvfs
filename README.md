# far-gvfs (gvfspanel)

Бета-версия, **использовать с осторожностью!**
Beta version, **use with care!**

_Только для особо интересующихся._
_Only for those who are particularly interested._

Очень простое дополнение к [far2l](https://github.com/elfmz/far2l). Обертка
вокруг GVFS: позволяет работать с сетевыми ресурсами как с виртуальными
файловыми системами. Поддерживаются те протоколы, которые поддерживает GVFS
(SFTP, WebDAV, SAMBA и др.).

Very simple plugin for far2l. Wrap around GVFS: allows you to work with
network resources as virtual file systems. Supported are those protocols
that support GVFS (SFTP, WebDAV, SAMBA, etc.).

#### Лицензия / License: MIT

Использован код из/Based on code from: https://github.com/invy/far2l/tree/gvfs/gvfspanel.

## Зависимости / Dependencies

* gtkmm-3.0 (возможно, будет работать и с более ранней 2 версией / may work
  with versions prior to 3);
* libuuid;
* OpenSSL версии 1.0.2 и старше (необязательная/optional);
* libsecret версии 0.18.5 и старше (необязательная/optional).

Установки зависимостей сборки в/Build dependencies in Debian (Ubuntu):

```
apt-get install libgtkmm-3.0-dev uuid-dev libssl-dev libsecret-1-dev
```

В зависимости от наличия libsecret и OpenSSL дополнение по-разному хранит
пароли. Если имеется libsecret, то пароли могут храниться в системных
безопасных хранилищах: Gnome Keyring или KDE Wallet. OpenSSL используется для
шифрования хранимых паролей, если безопасное хранилище не задействовано или
дополнение собрано без его поддержки. И наконец, если дополнение собирается
без OpenSSL и поддержки безопасного хранилища (или оно не используется), то
_пароли хранятся практически незашифрованными_!

Depending on the availability of libsecret and OpenSSL, the plugin support
different password storages. If libsecret is available, the passwords
can be stored in the system's secure storage: Gnome Keyring or KDE Wallet.
OpenSSL is used to encrypt stored passwords if a secure repository is not
involved or the plugin build without its support. Finally, if the plugin
build without OpenSSL and support for a secure storage (or it is not used),
then _the passwords are stored almost unencrypted_!

Шифрование хранимых паролей и использование системного безопасного хранилища
может быть отключено перед сборкой явно. Для этого в cmake-проекте
"CMakeLists.txt" дополнения имются соответствующие опции:
"STORE_ENCRYPTED_PASSWORDS" и "USE_SECRET_STORAGE". Если опция отключена, то
дополнение соберется без отключенной возможности, даже если нужные зависимости
доступны. И наоборот, если опция включена, то в отсутствие зависимостей сборка
дополнения прервется с ошибкой.

Для сборки дополнение помещается в дерево исходного кода far2l в виде
поддиректории. Hапример, если код far2l развернут в директорию "far2l",
то код far-gvfs помещается в "far2l/far-gvfs". Затем директорию добавляют
в проектный файл "CMakeLists.txt" far2l:

To build, the plugin is placed in the far2l source tree as a subdirectory.
For example, if the far2l code is expanded into the "far2l" directory, the
far-gvfs code is placed in "far2l / far-gvfs". Then the directory is added
to the project file "CMakeLists.txt" far2l:

```
...
add_subdirectory (align)
add_subdirectory (autowrap)
add_subdirectory (drawline)
add_subdirectory (far-gvfs)
...
```

Дополнение будет собрано и подготовлено к установке вместе с прочими
дополнениями far2l, входящими в его состав.

The plugin will be build and prepared for installation along with
other plugins far2l included in its composition.

Для работы дополнения в целевой ОС должны быть установлены:
To work the plugin in the target OS should be installed:

* gtkmm;
* поддержка GVFS;
* libuuid (пакет/package libuuid1 в/in Debian);
* libssl (необязательно/optional);
* libsecret (пакет/package libsecret-1, необязательно/optional).

## Использование / Using

Для монтирования сетевых ресурсов необходимо указать URL ресурса, учетную
запись и пароль для нужд аутентификации. Анонимное монтирование возможно:
попытки анонимного подключения выполняются, если и учетная запись, и пароль
_не указаны_. Вместо хранения пароля можно запрашивать его перед каждым
монтированием ресурса. В ходе подключения пользователю может быть сделан
запрос с выбором вариантов дальнейших действий. Формулировки запроса и
вариантов зависят от используемого протокола. Дополнение представляет их
в том виде, в каком получает от GVFS. Аналогично, в случае возникновения
каких-либо ошибок о них выдается сообщение, переданное GVFS. В случае удачного
завершения операции панель GVFS переключается на вновь подсоединенный ресурс.
Ресурсы в статусе подсоединенных в списке помечаются знаком "*". Отсоединить
ресурс можно, нажав Shift-F8. Статусы ресурсов на панели обновляются
автоматически, либо их можно обновить вручную, нажав Ctrl-R.

To mount network resources, you must specify the resource URL, account, and
password needed for authentication. Anonymous mounting is possible: anonymous
connections are attempted if neither the account or the password are
specified. Instead of storing the password, you can request it each time prior
to mounting the resource. During the connection, the user can make a request
with a choice of options for further action. The wording of the request and
options depends on the protocol used. The plugin presents them in the form
they receive from GVFS. Similarly, if any errors occur, a message is sent to
the GVFS. If the mount operation completes successfully, the GVFS panel will
then switch to the newly-connected resource. Connected resources are marked
with an asterisk ("\*") in the list. You can disconnect from the resource by
pressing Shift-F8. The status of each resource in the toolbar is updated
automatically, or you can update them manually by pressing Ctrl-R.

В зависимости от настроек дополнения, известные смонтированные ресурсы при
завершении работы с far2l можно автоматически отсоединять. Если запущено более
одного экземпляра far2l, то они пользуются общим (системным) пулом
смонтированных ресурсов. Таким образом, разрешение всех возможных конфликтов в
работе с ресурсами оставлено средствам самой GVFS.

Depending on the plugin's settings, the known mounted resources can be
disconnected automatically when you exit far2l. If more than one instance of
far2l is started, then they share a common (system) pool of mounted resources.
Thus, the resolution of all possible conflicts in working with resources is
left to GVFS.

Данные о сетевых ресурсах хранятся в реестре far2l в ветке
"Software/Far2/gvfspanel/Resources". В зависимости от настроек дополнения
пароли могут храниться отдельно в системных безопасных хранилищах. Если ранее
сохраненные данные перестали отображаться на панели или неправильно
загружаются, удалите ветку целиком и заведите ресурсы заново. Пароли в
системных хранилищах помечаются как "Far-gvfs password record". При удалении
ветки реестра рекомендуется вручную удалить и пароли в стороннем хранилище. В
случае, если пароль из стороннего хранилища извлечь не удалось, то запись о
ресурсе не отбрасывается. Пользователь сможет ввести пароль заново.

Data about network resources is stored in the far2l registry under the
"Software / Far2 / gvfspanel / Resources" branch. Depending on the plugin's
settings, passwords can be stored separately in the system's secure storage. If
the previously saved data is no longer displayed in the panel or is loaded
incorrectly, simply delete the entire branch and re-create the resources.
Passwords in system storage are marked as "Far-gvfs password record". When
deleting a registry branch, it is recommended that you manually delete the
passwords in the third-party repository. In the event that the password from
the third-party storage could not be retrieved, the resource record is not
discarded. The user can enter the password again.

Дополнение добавляется в меню выбора устройств far2l под именем "GVFS," а
также в меню конфигурации и команд дополнений. Параметры конфигурации far-gvfs /
Plugin is added to the far2l device selection menu under the name "GVFS," as
well as in the configuration menu and plugin commands. Far-gvfs configuration
parameters:

* Отсоединять или нет известные смонтированные ресурсы при выходе из far2l. По
  умолчанию отсоединяются. / disconnect or not known known resources when exiting
  far2l. By default, they are disconnected.
* Если ресурсы отсоединяются, то делать это только для ресурсов, смонтированных в
  текущем сеансе far2l, или для всех смонтированных.
* Использовать или нет для хранения паролей системные безопасные хранилища. По
  умолчанию не используются. Параметр может отсутствовать, если дополнение
  собрано без поддержки безопасных хранилищ. / use or not to store passwords for
  system-safe storage. By default not used. The parameter can be absent if the
  plugin build without the support of secure storages.

Команды/Commands:

* Переключение текущей панели на панель самого дополнения. / Switch the current
  panel to the panel of the plugin itself.

## Известные проблемы / Known issues

Симптомы|Вероятная причина|Комментарий
--------|-----------------|-----------
Ошибка 20005 при монтировании|Некоторые задержки в GVFS.|В основном безвредна, потому что ресурс фактически монтируется успешно. Подробности в [#14](https://github.com/cycleg/far-gvfs/issues/14).
Ошибка 2 при монтировании и переключение на локальную директорию|Указанного в URL ресурса не существует.|Домашняя директория аутентифицированного пользователя доступна, поэтому ресурс отображается как подсоединенный.
После подсоединения на панели отображается пустая директория.|У аутентифицированного пользователя нет прав на доступ к запрошенному ресурсу.|То же, что и в предыдущем случае.
Ошибка 2 при различных файловых операциях с использованием FTP в качестве транспорта.|Ошибки в реализации GVFS поверх FTP.| Это не проблема far-gvfs, потому что такие же проблемы проявляются и в других приложениях, работающих с GVFS.

Symptoms|Possible Cause|Comment
--------|--------------|-------
Error 20005 when mounting|Some delays in GVFS.|Basically harmless, because the resource is actually mounted successfully. Details at [#14](https://github.com/cycleg/far-gvfs/issues/14).
Error 2 when mounting and switching to local directory|The resource specified in the URL does not exist.|The home directory of the authenticated user is available, so the resource is displayed as connected.
After the connection, an empty directory is displayed on the panel.|An authenticated user does not have permission to access the requested resource.|The same as in the previous case.
Error 2 for various file operations using FTP as the transport.|Errors in the implementation of GVFS-over-FTP.|This is not a far-gvfs problem, because the same problems also occur in other applications that rely on GVFS.

## Документация / Documentation

Исходный код дополнения документирован с помощью doxygen. Для построения
документации достаточно отдать в корне дерева кода команду / The source
code for the plugin is documented using doxygen. For building documentation
it is enough to give the following command at the root of the code tree:

```
doxygen Doxyfile
```

Документация в формате HTML будет помещена в директорию:  docs/html/

Documentation in HTML format will be placed in the directory:  docs/html/
