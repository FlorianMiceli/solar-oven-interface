/*
Connection to backend server using axios
*/

import axios from 'axios'
import { error as displayError } from '@/helpers/display'

const baseURL = process.env.NODE_ENV === 'production' ? 'https://your-production-frontend-url.com' : 'http://localhost:8080/' //TODO replace by production url

let backend = axios.create({
    baseURL: baseURL
})

async function apiCall (method: string, url: string, data?: any, params?: any) {
    return await backend
        .request({ method, url, data, params })
        .then((response: any) => {
            return { 
                data: response.data, 
                status: true
            }
        })
        .catch((error: any) => {
            let error_message = ''
            if(error?.response?.data?.error_message){error_message = error?.response?.data?.error_message}
            else if(error?.message?.error_message){error_message = error?.message?.error_message}
            else if(error?.statusText){error_message = error?.statusText}
            else if(error?.message){error_message = error?.message}
            else{error_message = 'Unknown error'}
            if(error_message !== 'Network Error') displayError('Error', error_message)
            console.error(error_message)
            console.error(error)
            return { 
                data: null, 
                status: false,
                error_message: error_message
            }
        })
}

export async function apiGet(url: string, params?: any) {
  return await apiCall('get', url, {}, params)
}

export async function apiPost(url: string, data: any) {
    return await apiCall('post', url, data)
}

export async function apiPut(url: string, data: any) {
    return await apiCall('put', url, data)
}

export async function apiDelete(url: string, data: any) {
    return await apiCall('delete', url, data)
}
