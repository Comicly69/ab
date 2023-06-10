const express = require('express');
const mongodb = require('mongodb');

const MONGO_URI = '*';
const DB_NAME = 'user_info';
const COLLECTION_NAME = 'users';

const app = express();

app.use(express.urlencoded({ extended: true }));
app.use(express.json());

app.use(express.static(__dirname + '/public'));

app.get('/', (req, res) => {
  res.sendFile(__dirname + '/public/index.html');
});

app.post('/users', async (req, res) => {
  try {
    const { username, info } = req.body;

    const client = await mongodb.MongoClient.connect(MONGO_URI);
    const db = client.db(DB_NAME);
    const collection = db.collection(COLLECTION_NAME);

    await collection.insertOne({ username, info });

    client.close();

    res.sendStatus(200);
  } catch (error) {
    console.error('Error adding user data:', error);
    res.sendStatus(500);
  }
});

app.get('/search', async (req, res) => {
  try {
    const { username } = req.query;

    const client = await mongodb.MongoClient.connect(MONGO_URI);
    const db = client.db(DB_NAME);
    const collection = db.collection(COLLECTION_NAME);

    const user = await collection.findOne({ username });

    client.close();

    if (user) {
      const formattedInfo = user.info.replace(/\n/g, '<br>');

      res.send(`<h1>User Data for ${user.username}</h1><p>${formattedInfo}</p>`);
    } else {
      res.send(`<h1>Error</h1><p>User not found.</p>`);
    }
  } catch (error) {
    console.error('Error searching user data:', error);
    res.sendStatus(500);
  }
});

const port = 3000;
app.listen(port, () => {
  console.log(`Server running on http://localhost:${port}`);
});
