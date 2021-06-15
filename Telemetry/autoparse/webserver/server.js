const app 		    = require('express')();
const bodyparser 	= require('body-parser');
const busboy        = require('connect-busboy');
const child_process = require('child_process');
const fs			= require('fs');
const path          = require('path');

const PORT          = 5000;
const UPLOAD_PATH	= './uploads';

app.use(bodyparser.urlencoded({ extended: true }));

if (!fs.existsSync(UPLOAD_PATH))
	fs.mkdirSync(UPLOAD_PATH)

app.get('/', (undefined, res) => res.redirect('/index.html'));
app.get('/index.html', (req, res) => res.sendFile(path.resolve(__dirname, 'views/index.html')));
app.get('/upload.html', (req, res) => res.sendFile(path.resolve(__dirname, 'views/upload.html')));
app.get('/status.html', (req, res) => res.sendFile(path.resolve(__dirname, 'views/status.html')))

app.post('/upload', bodyparser.json(), busboy({ highWaterMark: 2 * 1024 * 1024 }), (req, res) => {
	var db_name, fstream, filename, filepath;

	req.busboy.on('field', (key, val) => {
		if (key == 'db_name')
			db_name = val.trim().replace(/ /g,"_");
	});

	req.busboy.on('file', (key, file, filename_) => {
		if (key == 'csv') {
			filename = filename_;
			filepath = path.join(UPLOAD_PATH, filename);
			fstream = fs.createWriteStream(filepath);
			console.log(`Upload of '${filename}' started`);
			file.pipe(fstream);
		}
	});

	req.busboy.on('finish', () => {
        if (!db_name || !fstream) {
			res.redirect('/upload.html?err=true');
			fs.unlinkSync(filepath);
			return;
		}
		fstream.on('close', () => {
            console.log(`Upload of '${filename}' finished`);
			res.send(`Your CSV is being written to database ${db_name}. Go to <a href="http://ec2-3-134-2-166.us-east-2.compute.amazonaws.com:3000"/>Grafana</a> to view`);

			child_process.exec('../exe/upload ' + db_name + ' ' + filepath, () => fs.unlinkSync(filepath))
        });
    });

	req.pipe(req.busboy);
});

// --- Everything past this point is related to configuration management --- //

const runScript = script => child_process.execSync('sh ~/update_scripts/' + script).toString()
const getScriptOutput = script => runScript(script).split('\n').map(x => x.trim()).filter(x => x.length);
const getBranches = () => getScriptOutput('branches.sh')

app.get('/config', (req, res) => { 
	let [ status, current ] = getScriptOutput('status.sh');
	let [ buildDate, ...parserIDs ] = getScriptOutput('list_canIDs.sh'); 
	res.json({ status, current, buildDate, parserIDs, branches: getBranches() }); 
})

app.post('/refresh', (req, res) => {
	const branch = req.body.new_branch;
	if (getBranches().includes(branch))
		runScript('restart.sh ' + branch).toString();
	res.redirect('/status.html');
})

app.listen(PORT, () => console.log("Server running on port " + PORT));
