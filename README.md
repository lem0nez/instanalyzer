# Instanalyzer
The tool for collecting and structuring information about the Instagram account. It based on the [Instaloader](https://github.com/instaloader/instaloader) and required the installed Python with version **3.5** or higher.\
Usage: `instanalyzer [parameters] {profile}`.

## Build
**Firstly**, you need to *configure* project with the following command: `./configure`. For collecting information about location you need also define variables in parameters:
* `HERE_APPID=<id>` and `HERE_APPCODE=<code>` to include support of the [**HERE** geocoder](https://developer.here.com/projects).
* `YANDEX_API_KEY=<key>` to include support of the [**Yandex** geocoder](https://developer.tech.yandex.ru).

You can provide both geocoders, but, mainly, only the **HERE** geocoder is enough.

**Then**, after configure, you need to *build dependencies*, *make project* and, optionally, *install* program in system:
```
make libs
make
sudo make install
```

## Main parameters
* `-i, --info` — get information about profile.
* `-l, --location` — show graphs with information about most recently visited places.
* `-c --commentators` — show most active commentators.
* `-o, --commentator` `<name>` — get information about commentator.
* `-p, --top-posts` `<count>` — show top of most liked posts. You can add prefix `r` before number of posts for reverse sorting.
* `-t, --tagged` — show often tagged profiles on pictures.
