"use strict"

const openwhisk = require('openwhisk');
const options = {
  api_key :
      '23bc46b1-71f6-4ed5-8c54-816aa4f8c502:123zO3xZCLrMN6v2BKK1dXYFpXlPkccOFqm12CdAsMgRU4VrNZ9lyGVCGuMDGIwP'
}

const ow = openwhisk(options)

const polling_delay = 500
const delay = async ms => new Promise(resolve => setTimeout(resolve, ms))

async function poll_act(activation) {
  let result = null
  do {
    try {
      result = await ow.activations.get({name : activation.activationId})
    } catch (err) {
      if (err.statusCode !== 404) {
        throw err
      }
    }

    if (!result) {
      await delay(polling_delay)
    }
  } while (!result);

  return result;
}

function output_file_name(start, end) {
  if (start == end || start == end - 1) {
    return '' + start + '.out'
  } else {
    return '' + start + '-' + end + '.out'
  }
}

async function mapreduce(params, start, end) {
  if (start == end || start == end - 1) {
    let oname = output_file_name(start, end);

    let activation = await ow.actions.invoke({
      name : 'count-words',
      params : {
        input_bucket : params.input_bucket,
        input_file : 'data' + start,
        output_bucket : params.output_bucket,
        output_file : oname
      }
    });

    let result = await poll_act(activation);
    return result;

  } else {
    const split = start + (end - start) / 2;

    // Launch two sub actions
    let result_first = mapreduce(params, start, split);
    let result_second = mapreduce(params, split, end);

    let reponse_first = await result_first;
    let reponse_second = await result_second;

    // Call reducer
    let oname = output_file_name(start, end);
    let iname_x = output_file_name(start, split);
    let iname_y = output_file_name(split, end);

    let activation = await ow.actions.invoke({
      name : 'merge-counts',
      params : {
        input_bucket : params.output_bucket,
        input_file_x : iname_x,
        input_file_y : iname_y,
        output_bucket : params.output_bucket,
        output_file : oname
      }
    });

    let result = await poll_act(activation);
    return result
  }
}

async function main(params) {
  const result = await mapreduce(params, params.start, params.end);
  return result;
}
