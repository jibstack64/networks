cd ~
git clone https://github.com/jibstack64/networks networksinstall
cd networksinstall

if ! type "gcc" > /dev/null; then
    echo Please install GCC/G++!
    cd ..
    rm -rf networksinstall
fi
g++ -std=c++17 networks.cpp -lstdc++fs -o networks.o
sudo mv networks.o /bin/networks
networks --help

cd ..
rm -rf networksinstall

echo All done!