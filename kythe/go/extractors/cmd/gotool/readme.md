# golang-extractor image

## Example usage

```.sh
mkdir -p /tmp/workspace/gopath/src/golang.org/x

git clone https://go.googlesource.com/tools /tmp/workspace/gopath/src/golang.org/x/tools

docker run \
  -v /tmp/workspace:/workspace:z \
  -e KYTHE_CORPUS=golang.org/x/tools \
  -e GOPATH=/workspace/gopath \
  -e OUTPUT=/workspace/out \
  gcr.io/kythe-public/golang-extractor:latest \
  golang.org/x/tools

# inspect output kzip
kzip info --input /tmp/workspace/out/compilations.kzip | jq
```
