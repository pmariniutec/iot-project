const { spawn } = require('child_process');
var multer  = require('multer');
const path = require('path');
// const client = require('../db/db.js');

const multerStorage = multer.diskStorage({
  destination: (req, file, cb) => {
    cb(null, './images');
  },
  filename: (req, file, cb) => {
    cb(null, `image-${Date.now()}` + path.extname(file.originalname))
    //path.extname get the uploaded file extension
  }
});

const multerFilter = (req, file, cb) => {
  if (!file.originalname.match(/\.(png|jpg)$/)) { 
    // upload only png and jpg format
    return cb(new Error('Please upload a Image'))
  }
  cb(null, true)
};

exports.upload = multer({
  storage: multerStorage,
  fileFilter: multerFilter
});

let runPyScript = function(imageName) {
  return new Promise(function(resolve, reject) {
    const pyprog = spawn('python', ['main.py', imageName]);
    console.log(`Processing image: ${imageName}`)
    pyprog.stdout.on('data', function(data) {
      resolve(data);
    });
    pyprog.stderr.on('data', (data) => {
      reject(data);
    });
  });
}

exports.uploadSingleImage = async (req, res) => {
  // const allquery = await client.query(`INSERT INTO users(name, icon) VALUES ('${req.body.filename}', '${req.file.filename}')`);
  runPyScript(`${req.file.destination}/${req.file.filename}`).then((data) => {
    const payload = data.toString()
    console.log(payload)
    res.status(200).json({ 'statusCode': 200, 'status': true, message: 'Image uploaded', 'data': [] });
  })
}
