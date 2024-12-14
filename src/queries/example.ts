import { apiGet } from '@/api'
import { createQueryKeys } from '@lukemorales/query-key-factory'

export const example = createQueryKeys('example', {
  example: {
    queryKey: null,
    queryFn: async () => {
      return (await apiGet("test/test")).data
    }
  }
})
