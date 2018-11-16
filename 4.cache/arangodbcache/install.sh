curl -OL https://download.arangodb.com/arangodb33/xUbuntu_17.04/Release.key
sudo apt-key add - < Release.key
echo 'deb https://download.arangodb.com/arangodb33/xUbuntu_17.04/ /' | sudo tee /etc/apt/sources.list.d/arangodb.list
sudo apt-get install apt-transport-https
sudo apt-get update
sudo apt-get install arangodb3=3.3.19
sudo apt-get install arangodb3-dbg=3.3.19
sudo service arangodb3 status
arangosh

db._create('docs');
db._collections();

db.docs.insert({_key: 'foo', value: 'bar'});
db.docs.insert({_key: 'foo1', value: 'bar1'});
db.docs.insert({_key: 'foo2', value: 'bar2'});
db.docs.insert({_key: 'foo3', value: 'bar3'});
db.docs.insert({_key: 'foo4', value: 'bar4'});

db._createDatabase("testdb");
db._useDatabase('testdb');
db._databases();


git clone https://github.com/arangodb/fuerte
sudo apt-get install libssl-dev
sudo apt-get install libboost-all-dev
sudo apt-get install libcurl4-openssl-dev
git clone https://github.com/arangodb/velocypack
cd velocypack
cmake --build .
make install

