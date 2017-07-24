/*
 * Copyright 2017 Google Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import * as Ajv from 'ajv';
import {accessSync} from 'fs';
import {resolve} from 'path';

import {PathConfig} from './pathContext';

export const SETTINGS_FILE = '.kythe-settings.json';

// Type of the data contained in the settings file
export type Settings = {
  mappings: PathConfig,
  xrefs: {host: string, port: number}
};

export function parseSettings(obj: {}): Settings|Error {
  const ajv = new Ajv({allErrors: true});

  if (!ajv.validate(settingsSchema, obj)) {
    const msg =
        (ajv.errors ||
         []).map(e => e.dataPath ? `${e.dataPath}: ${e.message}` : e.message);
    return new Error(`Settings Error: ${msg.join('; ')}`);
  }

  const settings = obj as Settings;
  settings.xrefs = {host: 'localhost', port: 8080, ...settings.xrefs};

  return settings;
}

// Searches upwards for the .kythe-settings.json file
export function findRoot(path: string): string|Error {
  while (true) {
    const possibleFile = resolve(path, SETTINGS_FILE);
    try {
      accessSync(possibleFile);
      return path;
    } catch (e) {
      const newPath = resolve(path, '..');
      if (newPath === path) {
        return new Error('.kythe-settings.json file not found');
      }
      path = newPath;
    }
  }
}

// JSON Schema for settings
const settingsSchema = {
  'additionalProperties': false,
  'properties': {
    'mappings': {
      'items': {
        'additionalProperties': false,
        'properties': {
          'local': {'type': 'string'},
          'vname': {
            'additionalProperties': false,
            'required': ['path'],
            'properties': {
              'corpus': {'type': 'string'},
              'path': {'type': 'string'},
              'root': {'type': 'string'}
            },
            'type': 'object'
          }
        },
        'required': ['local', 'vname'],
        'type': 'object'
      },
      'minItems': 1,
      'type': 'array'
    },
    'xrefs': {
      'additionalProperties': false,
      'properties': {'host': {'type': 'string'}, 'port': {'type': 'integer'}},
      'required': ['host', 'port'],
      'type': 'object'
    }
  },
  'required': ['mappings'],
  'type': 'object'
};
