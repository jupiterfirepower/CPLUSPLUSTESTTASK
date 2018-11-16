curl -OL https://download.arangodb.com/arangodb33/xUbuntu_17.04/Release.key
sudo apt-key add - < Release.key
echo 'deb https://download.arangodb.com/arangodb33/xUbuntu_17.04/ /' | sudo tee /etc/apt/sources.list.d/arangodb.list
sudo apt-get install apt-transport-https
sudo apt-get update
sudo apt-get install arangodb3=3.3.19
sudo apt-get install arangodb3-dbg=3.3.19
sudo service arangodb3 status
arangosh

git clone https://github.com/arangodb/fuerte
sudo apt-get install libssl-dev
sudo apt-get install libboost-all-dev
sudo apt-get install libcurl4-openssl-dev
git clone https://github.com/arangodb/velocypack
cmake --build .

