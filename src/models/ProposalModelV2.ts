import {
  BaseModel,
  Metadata,
  UInt8,
  UInt32,
  XFL,
  Hash256,
  Currency,
  XRPAddress,
} from '@transia/hooks-toolkit/dist/npm/src/libs/binary-models'

export class ProposalModelV2 extends BaseModel {
  status: UInt8 // 0 open 1 closed
  expiration: UInt32 // Last Ledger Sequence
  orderType: UInt8 // 0 BUY 1 SELL
  quantity: XFL // Quantity as XFL
  price: XFL // Price as XFL
  currency: Currency // Currency as string
  issuer: XRPAddress // Issuer as xrp r address
  id: Hash256 // Hash256

  constructor(
    status: UInt32,
    expiration: UInt32,
    orderType: UInt8,
    quantity: XFL,
    price: XFL,
    currency: Currency,
    issuer: XRPAddress,
    id: Hash256
  ) {
    super()
    this.status = status
    this.expiration = expiration
    this.orderType = orderType
    this.quantity = quantity
    this.price = price
    this.currency = currency
    this.issuer = issuer
    this.id = id
  }

  getMetadata(): Metadata {
    return [
      { field: 'status', type: 'uint8' },
      { field: 'expiration', type: 'uint32' },
      { field: 'orderType', type: 'uint8' },
      { field: 'quantity', type: 'xfl' },
      { field: 'price', type: 'xfl' },
      { field: 'currency', type: 'currency' },
      { field: 'issuer', type: 'xrpAddress' },
      { field: 'id', type: 'hash256' },
    ]
  }

  toJSON() {
    return {
      status: this.status,
      expiration: this.expiration,
      orderType: this.orderType,
      quantity: this.quantity,
      price: this.price,
      currency: this.currency,
      issuer: this.issuer,
      id: this.id,
    }
  }
}
