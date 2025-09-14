Build skeleton docker:
```
./skeleton-build.sh
```

Build all actions:
```
docker run -it -v $PWD:/action/ -w /action/ yhdengh/fixpoint-benchmark:skeleton ./build.sh
```

Build container-based actions:
```
./docker-action-build.sh
```

Build zip-based actions:
```
./zip-actions.sh
```
