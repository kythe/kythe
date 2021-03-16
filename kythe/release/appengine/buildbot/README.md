# Description

AppEngine configuration for Kythe's Buildbot instance.

https://buildbot-dot-kythe-repo.appspot.com/

# Dependencies

* [buildbot](https://buildbot.net/) (≥v1.3.0; to check config before deployment)
  * `pip install buildbot txrequests`
* [docker](https://www.docker.com/)
* [gcloud](https://cloud.google.com/sdk/gcloud/)

# Deployment

1) Make sure `gcloud` works in the context of the `kythe-repo` project:

```shell
gcloud components update
gcloud config set project kythe-repo
```

2) Run the `deploy.sh` script

```shell
./kythe/releases/appengine/buildbot/deploy.sh
```
