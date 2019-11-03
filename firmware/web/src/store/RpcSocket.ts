/** @file
 * RPC Socket - Handles JSON-RPC over WebSockets
 *
 * \copyright Copyright (c) 2018 Chris Byrne. All rights reserved.
 * Licensed under the MIT License. Refer to LICENSE file in the project root.
 */
import { CombinedVueInstance } from 'vue/types/vue';
import Vue from 'vue';

type IJsonRpcParamsPrimitives = object | string;
type IJsonRpcParams = IJsonRpcParamsPrimitives[] | IJsonRpcParamsPrimitives;
type IJsonRpcResult = IJsonRpcParams;

interface IJsonRpcError {
  /**
   * A Number that indicates the error type that occurred.
   * This MUST be an integer.
   */
  code: number;

  /**
   * A String providing a short description of the error.
   * The message SHOULD be limited to a concise single sentence.
   */
  message: string;

  /**
   * A Primitive or Structured value that contains additional information about the error.
   * This may be omitted.
   * The value of this member is defined by the Server (e.g. detailed error information,
   * nested errors etc.).
   */
  data?: any;
}

interface IJsonRpcRequest {
  /**
   * A String specifying the version of the JSON-RPC protocol. MUST be exactly "2.0"
   */
  jsonrpc: string;

  /**
   * A String containing the name of the method to be invoked. Method names that begin
   * with the word rpc followed by a period character (U+002E or ASCII 46) are reserved
   * for rpc-internal methods and extensions and MUST NOT be used for anything else.
   */
  method: string;

  /**
   * A Structured value that holds the parameter values to be used during the invocation
   * of the method. This member MAY be omitted.
   */
  params?: IJsonRpcParams;

  /**
   * An identifier established by the Client that MUST contain a String, Number, or NULL
   * value if included. If it is not included it is assumed to be a notification. The
   * value SHOULD normally not be Null [1] and Numbers SHOULD NOT contain fractional parts [2]
   */
  id: string | number | null;
}

interface IJsonRpcRequest {
  /**
   * A String specifying the version of the JSON-RPC protocol. MUST be exactly "2.0"
   */
  jsonrpc: string;

  /**
   * This member is REQUIRED on success.
   * This member MUST NOT exist if there was an error invoking the method.
   * The value of this member is determined by the method invoked on the Server.
   */
  result: any;

  /**
   * This member is REQUIRED on error.
   * This member MUST NOT exist if there was no error triggered during invocation.
   * The value for this member MUST be an Object as defined in section 5.1.
   */
  error: IJsonRpcError;

  /**
   * This member is REQUIRED.
   * It MUST be the same as the value of the id member in the Request Object.
   * If there was an error in detecting the id in the Request object
   * (e.g. Parse error/Invalid Request), it MUST be Null.
   */
  id: string | number | null;
}

interface IPendingRequest {
  resolve: (result: IJsonRpcResult) => void;
  reject: (error: Error) => void;
}

export default class {
  private connected_ = false;
  private connectTimer_?: NodeJS.Timer;
  private debug_ = false;
  private eventHub_: CombinedVueInstance<Vue, { connected: boolean }, object, object, any>;
  private heartbeatInterval_ = 3000; // ms between heartbeats
  private heartbeatTimer_?: NodeJS.Timer;
  private nextId_ = 0;
  private reconnectDelay_ = 0;
  private pending_: { [key: string]: IPendingRequest } = { };
  private sawActivity_ = false;
  private state_: any = { };
  private timerId_?: NodeJS.Timer;
  private url_: string;
  private ws_?: WebSocket;

  constructor(path = '') {
    this.url_ = `ws://${document.location.host}${path}`;

    this.eventHub_ = new Vue({
      data: {
        connected: true, // initially assumes we have a connection
      },
    });
  }

  public connect() {
    this.debugLog_(`connecting to ${this.url_} ...`);
    this.ws_ = new WebSocket(this.url_);
    this.ws_.onopen = () => this.onConnected_();
    this.ws_.onclose = () => this.onDisconnect_();
    this.ws_.onerror = (error: Event) => this.onError_(error);
    this.ws_.onmessage = (m) => this.onMessage_(JSON.parse(m.data));

    this.connectTimer_ = setTimeout(async () => {
      if (!this.connected_) {
        // we haven't been able to connect
        this.eventHub_.$emit('disconnect');
      }
    }, 1500);
  }

  public request(method: string, params?: any) {
    if (!this.connected_) {
      throw new Error('Not connected');
    }

    return new Promise((resolve, reject) => {
      if (!this.ws_) {
        reject(new Error('No WebSocket'));
        return;
      }

      const id = this.nextId_++;
      this.ws_.send(JSON.stringify({
        jsonrpc: '2.0',
        id,
        method,
        params,
      }));
      this.pending_[id] = { resolve, reject };
    });
  }

  get connected() {
    return this.eventHub_.connected;
  }
  get state() {
    return this.state_;
  }

  public on(event: string | string[], callback: (...args: any[]) => void) {
    return this.eventHub_.$on(event, callback);
  }
  public off(event: string | string[], callback?: (...args: any[]) => void) {
    return this.eventHub_.$off(event, callback);
  }

  private async onConnected_() {
    this.debugLog_('connected');
    if (this.connectTimer_) {
      clearTimeout(this.connectTimer_);
      this.connectTimer_ = undefined;
    }
    this.connected_ = true;
    this.reconnectDelay_ = 0;
    this.eventHub_.connected = true;
    this.nextId_ = 0;
    this.beginHeartbeat_();
    this.state_ = await this.request('state');
    this.eventHub_.$emit('connect', this.state_);
  }
  private onDisconnect_() {
    // skip if reconnect is already in progress
    if (!this.connected_ && this.timerId_) { return; }

    this.debugLog_('disconnected');
    this.eventHub_.connected = false;
    this.connected_ = false;

    if (this.heartbeatTimer_) {
      clearTimeout(this.heartbeatTimer_);
      this.heartbeatTimer_ = undefined;
    }

    // notify pending requests
    Object.keys(this.pending_).forEach((k) => {
      this.pending_[k].reject(new Error('Disconnected'));
    });
    this.pending_ = {};
    this.eventHub_.$emit('disconnect');

    // delay before retrying connection
    this.reconnectDelay_ = Math.min(Math.max(500, this.reconnectDelay_ * 2), 5000);
    this.timerId_ = setTimeout(() => {
      this.timerId_ = undefined;
      this.connect();
    }, this.reconnectDelay_);
  }
  private onError_(error: Event) {
    this.debugLog_('onerror', error);
  }
  private onMessage_(msg: Partial<IJsonRpcRequest>) {
    this.debugLog_('RPC', msg);
    if (msg.jsonrpc !== '2.0') {
      return;
    }

    // received data on socket, so we can delay any heartbeat
    this.sawActivity_ = true;

    // notify any pending request
    const promise = (null != msg.id) && this.pending_[msg.id];
    if (promise) {
      if (null != msg.id) {
        delete this.pending_[msg.id];
      }
      if (null != msg.error) {
        promise.reject(new Error(msg.error.message));
      } else {
        promise.resolve(msg.result);
      }
    }

    // non-solicited update
    if (msg.method === 'update') {
      this.eventHub_.$emit('update', msg.params);
    }
  }

  private beginHeartbeat_() {
    this.heartbeatTimer_ = setTimeout(async () => {
      if (!this.connected_) {
        return;
      }

      // don't send heartbeat if we're receiving other responses
      if (this.sawActivity_) {
        this.sawActivity_ = false;
        this.beginHeartbeat_();
        return;
      }

      //
      try {
        await Promise.race([
          new Promise((resolve, reject) => {
            setTimeout(() => reject(new Error('timed out')), 3000);
          }),
          this.request('ping'),
        ]);
        this.beginHeartbeat_();
      } catch (e) {
        this.debugLog_('heartbeat', e);
        if (null != this.ws_) {
          this.ws_.close();
          this.ws_ = undefined;
        }
        this.onDisconnect_();
      }
    }, this.heartbeatInterval_);
  }

  /**
   * debug controllable console logging
   * @param args arguments to log to console
   */
  private debugLog_(...args: any[]) {
    if (this.debug_) { console.log(...args); } // tslint:disable-line no-console
  }
}
