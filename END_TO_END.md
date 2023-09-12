```
docker build -t bpfilter .
docker run --privileged bpfilter
```

# TODO
- [ ] Uncomment `--bpf`
- [ ] Uncomment `self.assertEqual(stats[0]['rules'][0]['bytes'], len(icmp))`
- [ ] Add linting (mandatory docstring)
- [ ] Use network namespaces
