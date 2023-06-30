import {
  BaseModel,
  Metadata,
  UInt8,
  UInt64,
} from '@transia/hooks-toolkit/dist/npm/src/libs/binary-models'

export class VoteModel extends BaseModel {
  id: UInt64
  position: UInt8

  constructor(id: UInt64, position: UInt8) {
    super()
    this.id = id
    this.position = position
  }

  getMetadata(): Metadata {
    return [
      { field: 'id', type: 'uint64' },
      { field: 'position', type: 'uint8' },
    ]
  }

  toJSON() {
    return {
      id: this.id,
      position: this.position,
    }
  }
}
