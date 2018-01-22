# nvidia-settings with added nvspeedup.c
- allow to increase core and memory speed directly from console via added nvspeedup.c using nvlink proto via Xorg nvidia driver

Usage, to increase individual GPUs memory and core speeds:
```
./nvspeedup 1811 -100 1500 -100 1500 -100 1500 -100 1500 -100
```

example output:
```
- connecting to display ':0'
- OK.

- getting active system
- OK.

- getting target..
- OK.

- listing GPUs..

- [0] GeForce GTX 1060 6GB
        vbios:86.06.39.00.6f ram:6144M cores:1280 pci-e:1x (2500 Gbps) perfstate:P2
        mem     => +1811 MHz    [OK]
        core    => +-100 MHz    [OK]

- [1] GeForce GTX 1060 6GB
        vbios:86.06.39.00.6f ram:6144M cores:1280 pci-e:1x (2500 Gbps) perfstate:P2
        mem     => +1500 MHz    [OK]
        core    => +-100 MHz    [OK]

- [2] GeForce GTX 1060 6GB
        vbios:86.06.0e.00.a4 ram:6144M cores:1280 pci-e:1x (2500 Gbps) perfstate:P2
        mem     => +1500 MHz    [OK]
        core    => +-100 MHz    [OK]

- [3] GeForce GTX 1060 6GB
        vbios:86.06.39.00.6f ram:6144M cores:1280 pci-e:1x (2500 Gbps) perfstate:P2
        mem     => +1500 MHz     [OK]
        core    => +-100 MHz     [OK]

- [4] GeForce GTX 1060 6GB
        vbios:86.06.27.00.9b ram:6144M cores:1280 pci-e:1x (2500 Gbps) perfstate:P2
        mem     => +1500 MHz     [OK]
        core    => +-100 MHz     [OK]
```

*you need running Xorg server with Nvidia driver installed*
