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

import {RequestAPI, UrlOptions} from 'request';
import {defaults, RequestPromise, RequestPromiseOptions} from 'request-promise-native';

import {kythe} from './proto/xref';

export interface XRefClient {
  decorations(req: kythe.proto.IDecorationsRequest):
      Promise<kythe.proto.IDecorationsReply>;

  xrefs(req: kythe.proto.ICrossReferencesRequest):
      Promise<kythe.proto.ICrossReferencesReply>;
}

export class XRefHTTPClient implements XRefClient {
  // An HTTP client preconfigured with all shared parameters
  private client: RequestAPI<RequestPromise, RequestPromiseOptions, UrlOptions>;

  constructor(host: string, port: number) {
    this.client = defaults(
        {baseUrl: `http://${host}:${port}/`, method: 'POST', json: true});
  }

  async decorations(req: kythe.proto.IDecorationsRequest):
      Promise<kythe.proto.IDecorationsReply> {
    try {
      return await this.client('decorations', {body: req});
    } catch (e) {
      console.error(e.message);
      return {};
    }
  }

  async xrefs(req: kythe.proto.ICrossReferencesRequest):
      Promise<kythe.proto.ICrossReferencesReply> {
    try {
      return await this.client('xrefs', {body: req});
    } catch (e) {
      console.error(e.message);
      return {};
    }
  }
}
