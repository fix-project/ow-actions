"use strict"

const openwhisk = require('openwhisk');
const options = {
  api_key :
      '23bc46b1-71f6-4ed5-8c54-816aa4f8c502:123zO3xZCLrMN6v2BKK1dXYFpXlPkccOFqm12CdAsMgRU4VrNZ9lyGVCGuMDGIwP'
}

const ow = openwhisk(options)

const polling_delay = 100
const delay = async ms => new Promise(resolve => setTimeout(resolve, ms))

async function poll_act(activation) {
  let result = null
  try{
    do {
      try {
        result = await ow.activations.get({name : activation.activationId})
      } catch (err) {
      }

      if (!result) {
        await delay(polling_delay)
      }
    } while (!result);
  } catch (uncatchederr) {
    console.error( uncatchederr );
    throw uncatchederr
  }

  return result;
}

const invoke_delay = 100;
async function try_submit(activation) {
	let result = null
	try {
      	  do {
      	  	try { 
      	  		result = await ow.actions.invoke(activation);
      	  	} catch (err) {
      	  	}
      
      	  	if (!result) {
      	  		await delay(invoke_delay)
      	  	}
      	  } while (!result);
	} catch ( uncatchederr ) {
          console.error( uncatchederr );
          throw uncatchederr
	}

	return result;
}

function output_file_name(start, end) {
  if (start == end || start == end - 1) {
    return '' + start + '.out'
  } else {
    return '' + start + '-' + end + '.out'
  }
}

var Minio = require('minio');

var minioClient = new Minio.Client({
	endPoint: '10.103.170.222',
	port: 80,
	useSSL: false,
	accessKey: 'minioadmin',
	secretKey: 'minioadmin',
});

const fs = require('fs');

async function create_count_words(index) {
	const name = 'count-words-' + index.toString()

	try {
		const result = await ow.actions.get( name );
	} catch ( err ) {
		const get_count_words = await minioClient.fGetObject('actions', 'count-words.zip', 'count-words.zip')
		
		const countwords = fs.readFileSync('count-words.zip')

		try {
			const result = await ow.actions.create({
				"namespace": "guest",
				"name": name,
				"action": countwords,
				"exec": {
					"kind": "blackbox",
					"image": "openwhisk/dockerskeleton",
					"binary": true
				},
				"annotations": [
					{
						"key": "provide-api-key",
						"value": false
					},
					{
						"key": "exec",
						"value": "blackbox"
					}
				],
				"limits": {
					"timeout": 240000,
					"memory": 4096,
					"logs": 10,
					"concurrency": 1
				}
			});
		} catch ( newerr ) {
			console.log( 'action concurrently created' );
		}
	}

	return name
}

async function create_merge_counts(index) {
	const name = 'merge-counts-' + index.toString()

	try {
		const result = await ow.actions.get( name );
	} catch ( err ) {
		const get_count_words = await minioClient.fGetObject('actions', 'merge-counts.zip', 'merge-counts.zip')
		
		const mergecounts = fs.readFileSync('merge-counts.zip')

		try {
			const result = await ow.actions.create({
				"namespace": "guest",
				"name": name,
				"action": mergecounts,
				"exec": {
					"kind": "blackbox",
					"image": "openwhisk/dockerskeleton",
					"binary": true
				},
				"annotations": [
					{
						"key": "provide-api-key",
						"value": false
					},
					{
						"key": "exec",
						"value": "blackbox"
					}
				],
				"limits": {
					"timeout": 240000,
					"memory": 256,
					"logs": 10,
					"concurrency": 1
				}
			});
		} catch ( newerr ) {
			console.log( 'action concurrently created' );
		}
	}
	return name
}

async function mapreduce(params, start, end, count_words, merge_counts) {
  if (start == end || start == end - 1) {
    let oname = output_file_name(start, end);

    let activation = await try_submit({
      name : count_words,
      params : {
        input_bucket : params.input_bucket,
        input_file : 'chunk' + start,
	query: "A27",
	minio_url: "10.103.170.222:80",
      }
    });

    let result = await poll_act( activation );
    return result.response.result;
  } else {
    const split = start + Math.floor( (end - start) / 2 );

    let promise_left = mapreduce(params, start, split, count_words, merge_counts);
    let promise_right = mapreduce(params, split, end, count_words, merge_counts);
    let result_left = null
    let result_right = null

    try 
    {
      result_left = await promise_left;
    } catch (err) {
      console.error( err )
    }

    try 
    {
      result_right = await promise_right;
    } catch (err) {
      console.error( err )
    }

    let activation = await try_submit({
      name : merge_counts,
      params : {
	input_x : result_left,
	input_y : result_right,
      }
    });

    let result = await poll_act( activation );
    return result.response.result;
  }
}

async function mymapreduce(params) {
  const count_words_created = await create_count_words(params.index);
  const merge_counts_created = await create_merge_counts(params.index);

  const now = Date.now();
  console.log(`Action created: `, now);

  const result = await mapreduce(params, params.start, params.end, count_words_created, merge_counts_created);
  return result;
}

exports.main = mymapreduce
